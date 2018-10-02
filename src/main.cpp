/** MBED headers */
#include "mbed.h"
#include "rtos.h"
#include "EthernetInterface.h"
#include "TCPServer.h"
#include "TCPSocket.h"
#include "lpc_phy.h"
#include "watchdog.h"

/* IAP includes */
#include "boot.h"
#include "IAP.h"
/* Drivers */
#include "PID.h"
#include "pcbnAPI.h"
#include "Drivers.h"
#include "cli.h"
#include "util.h"

/** BSMP headers */
extern "C" {
#include "server_priv.h"
#include "server.h"
#include <bsmp/server.h>
}

#define DP8_SPEED10MBPS (1 << 1)    /**< 1=10MBps speed */
#define DP8_VALID_LINK  (1 << 0)    /**< 1=Link active */

#define BUFSIZE         256
#define SERVER_PORT     6791

/* PID Constants */
#define PID_RATE        1.0
#define PID_OUTMAX      3.3
#define PID_OUTMIN      0.0

#define FILE_DATASIZE   128

/* Firmware version macros */
#define FW_VERSION      "V1_1_0"

// BSMP Variables arrays
double Att[1];
double TempAC[1];
double TempBD[1];
double Set_PointAC[1];
double Set_PointBD[1];
uint8_t Temp_Control[1];
double HeaterAC[1];
double HeaterBD[1];
uint8_t Reset[1];
uint8_t Reprogramming[1];
uint8_t Data[FILE_DATASIZE];
char Version[8];
double PID_AC_Kc[1];
double PID_AC_tauI[1];
double PID_AC_tauD[1];
double PID_BD_Kc[1];
double PID_BD_tauI[1];
double PID_BD_tauD[1];
char IP_Addr[16];
char Mask_Addr[16];
char Gateway_Addr[16];
char MAC_Addr[18];

char mac_buffer[6];

#define READ_ONLY  0
#define READ_WRITE 1

#define RFFE_VAR( data, rw ) { { 0, rw, sizeof(data) }, NULL, data, NULL }
/* The index in this table will coincide with the index on the server list, since it registrates the variables sequentially */

struct bsmp_var rffe_vars[] = {
    /* [0] = */  RFFE_VAR( Att,            READ_WRITE ), // Attenuators
    /* [1] = */  RFFE_VAR( TempAC,         READ_ONLY ), // TempAC
    /* [2] = */  RFFE_VAR( TempBD,         READ_ONLY ), // TempBD
    /* [3] = */  RFFE_VAR( Set_PointAC,    READ_WRITE ), // Set_PointAC
    /* [4] = */  RFFE_VAR( Set_PointBD,    READ_WRITE ), // Set_PointBD
    /* [5] = */  RFFE_VAR( Temp_Control,   READ_WRITE ), // Temp_Control
    /* [6] = */  RFFE_VAR( HeaterAC,       READ_WRITE ), // HeaterAC
    /* [7] = */  RFFE_VAR( HeaterBD,       READ_WRITE ), // HeaterBD
    /* [8] = */  RFFE_VAR( Reset,          READ_WRITE ), // Reset
    /* [9] = */  RFFE_VAR( Reprogramming,  READ_WRITE ), // Reprogramming
    /* [10] = */ RFFE_VAR( Data,           READ_WRITE ), // Data
    /* [11] = */ RFFE_VAR( Version,        READ_ONLY ), // Version
    /* [12] = */ RFFE_VAR( PID_AC_Kc,      READ_WRITE ), // PID_AC_Kc
    /* [13] = */ RFFE_VAR( PID_AC_tauI,    READ_WRITE ), // PID_AC_tauI
    /* [14] = */ RFFE_VAR( PID_AC_tauD,    READ_WRITE ), // PID_AC_tauD
    /* [15] = */ RFFE_VAR( PID_BD_Kc,      READ_WRITE ), // PID_BD_Kc
    /* [16] = */ RFFE_VAR( PID_BD_tauI,    READ_WRITE ), // PID_BD_tauI
    /* [17] = */ RFFE_VAR( PID_BD_tauD,    READ_WRITE ), // PID_BD_tauD
    /* [18] = */ RFFE_VAR( IP_Addr,        READ_WRITE ), // Ip Address
    /* [19] = */ RFFE_VAR( MAC_Addr,       READ_WRITE ), // MAC Address
    /* [20] = */ RFFE_VAR( Gateway_Addr,   READ_WRITE ), // Gateway Address
    /* [21] = */ RFFE_VAR( Mask_Addr,      READ_WRITE ), // Mask Address
};

typedef struct {
    struct bsmp_raw_packet msg;
    Mail<struct bsmp_raw_packet, 5> *response_mail_box;
} bsmp_mail_t;

Mail<bsmp_mail_t, 5> bsmp_mail_box;
Mail<struct bsmp_raw_packet, 5> eth_mail_box;

/* Setup the watchdog timer */
Watchdog wdt;

/* BSMP server */
bsmp_server_t *bsmp;

// Threads
Thread Temp_Control_thread(osPriorityNormal, 1200, NULL, "TEMP");
Thread Attenuators_thread(osPriorityNormal, 800, NULL, "ATT");
Thread CLI_Proccess_Thread(osPriorityNormal, 1024, NULL, "CLI");
Thread BSMP_Thread(osPriorityNormal, 800, NULL, "BSMP");

// Hardware Initialization - MBED

// MBED Leds
DigitalOut led1(P1_18);
DigitalOut led2(P1_20);
DigitalOut led3(P1_21);
DigitalOut led4(P1_23);

// MBED pins
DigitalOut dataC(P0_1); // Data line to attenuator. LVTTL, low = reset, init = low.Set first attenuators (Att)
DigitalOut dataA(P0_18); // Data line to attenuator. LVTTL, low = reset, init = low.Set first attenuators (Att)
DigitalOut clk(P0_17); // Digital control attenuation. LVTTL, low = reset, init = low.Digital control attenuation
DigitalOut LE(P0_15); // Chip select for RFFE attenuators (all channels). LVTTL, low = reset, init = low.Digital control calibration
DigitalOut CSac(P2_1); // Chip select for ADT7320UCPZ-R2. LVTTL, high = disable, init = high.Temp. measurement in RFFE_AC
DigitalOut SHDN_temp(P0_23); // Shut down the temperature current boost output amplifier. LVTTL, low = disable, init = low.
DigitalOut led_g(P1_30); // Green LED
DigitalOut led_r(P1_31); // Red LED
DigitalOut CSbd(P0_16); // Chip select for ADT7320UCPZ-R2. LVTTL, high = disable, init = high.Temp. measurement in RFFE_BD
DigitalOut dataB(P2_0); // Data line to attenuator. LVTTL, low = reset, init = low. Set RF attenuators (Att)
DigitalOut dataD(P0_11); // Data line to attenuator. LVTTL, low = reset, init = low. Set RF attenuators (Att)
DigitalOut CS_dac(P0_24); // Chip select for DAC. LVTTL, low = Selected, init = high.Chip select
RawSerial pc(P0_2, P0_3); // Serial USB port. (NOTE: All printf() calls are redirected to this port)
SPI spi1(P0_9,P0_8,P0_7); //SPI Interface - spi(mosi, miso, sclk)

I2C pll_i2c(P0_27, P0_28);
CDCE906 pll(pll_i2c, 0b11010010);
I2C feram_i2c(P0_19, P0_20);
DigitalOut feram_wp(P0_21);
FeRAM feram(feram_i2c, feram_wp);

/* MBED functions replacements */
extern "C" void mbed_mac_address(char *s)
{
    memcpy(s, mac_buffer, 6);
}

/* MBED Reset function */
void mbed_reset( void )
{
    wdt.kick(0.1);
    /* Lock the firmware and wait for the overflow */
    while(1);
}

void wdt_heartbeat( void )
{
    /* If we don't receive a new message from the BSMP client in the next 10s, reset */
    /* This is a VERY dirty workaround for the hanging TCP bug, it's temporary so we don't stop the comissioning */
    wdt.kick(10.0);
}

bool get_eth_link_status(void)
{
    return (lpc_mii_read_data() & DP8_VALID_LINK) ? true : false;
}

void Temp_Feedback_Control( void )
{
    /* Temperature Sensors */
#if defined(TEMP_SENSOR_LM71)
    LM71 AC_Temp_sensor( spi1, CSac, 1000000, LM71_MODE_CONVERSION, 0.0, 100.0 );
    LM71 BD_Temp_sensor( spi1, CSbd, 1000000, LM71_MODE_CONVERSION, 0.0, 100.0 );
#elif defined(TEMP_SENSOR_ADT7320)
    ADT7320 AC_Temp_sensor( spi1, CSac, 1000000, ADT7320_CFG_16_BITS, 0, 0.0, 100.0 );
    ADT7320 BD_Temp_sensor( spi1, CSbd, 1000000, ADT7320_CFG_16_BITS, 0, 0.0, 100.0 );
#else
#error "No temperature sensor selected. Define it with the macros 'TEMP_SENSOR_LM71' or 'TEMP_SENSOR_ADT7320'"
#endif

    /* Heater DAC output */
    DAC7554 AC_Heater_DAC( spi1, CS_dac, DAC_AC_SEL, 3.3 );
    DAC7554 BD_Heater_DAC( spi1, CS_dac, DAC_BD_SEL, 3.3 );

    /* Temperature PIDs */
    double SetP_AC, SetP_BD;
    double ProcessValueAC, ProcessValueBD;
    double voutAC, voutBD;

    PID pidAC( &ProcessValueAC, &voutAC, &SetP_AC, get_value64(PID_AC_Kc), get_value64(PID_AC_tauI), get_value64(PID_AC_tauD), DIRECT );
    PID pidBD( &ProcessValueBD, &voutBD, &SetP_BD, get_value64(PID_BD_Kc), get_value64(PID_BD_tauI), get_value64(PID_BD_tauD), DIRECT );

    int state = 2;
    int pid_state = MANUAL;

    printf("Initializing Temp Control thread\n\r");

    /* Create PIDs with generic tuning constants (they will be updated as soon as the control loop starts) */
    pidAC.SetSampleTime( PID_RATE*1000 );
    //pidAC.SetInputLimits( 0.0 , 100.0 );
    pidAC.SetOutputLimits( PID_OUTMIN , PID_OUTMAX );
    pidAC.SetMode( pid_state ); // Start with the automatic control disabled

    pidBD.SetSampleTime( PID_RATE*1000 );
    //pidBD.SetInputLimits( 0.0 , 100.0 );
    pidBD.SetOutputLimits( PID_OUTMIN, PID_OUTMAX );
    pidAC.SetMode( pid_state ); // Start with the automatic control disabled

    SHDN_temp = 1;

    while (1) {
        if (state != get_value8(Temp_Control)) {
            printf ("Temperature control in %s mode!\n\r", (get_value8(Temp_Control) == AUTOMATIC) ? "AUTOMATIC":"MANUAL");
            state = get_value8(Temp_Control);

            pid_state = (state != MANUAL) ? AUTOMATIC : MANUAL;

            pidAC.SetMode( pid_state );
            pidBD.SetMode( pid_state );
        }

        // Read values from RFFE's temperature sensors
        set_value(TempAC,AC_Temp_sensor.Read());
        set_value(TempBD,BD_Temp_sensor.Read());

        // Update the Process Values
        ProcessValueAC = get_value64(TempAC);
        ProcessValueBD = get_value64(TempBD);

        /* Update Set Points */
        SetP_AC = get_value64(Set_PointAC);
        SetP_BD = get_value64(Set_PointBD);

#ifdef DEBUG_PRINTF
        printf( "AC_Temp = %f \n\r", ProcessValueAC );
        printf( "BD_Temp = %f \n\r", ProcessValueBD );
        printf( "PID_AC Params:\n\r");
        printf( "\tKc:%f\ttauI:%f\ttauD:%f\n\r", get_value64(PID_AC_Kc), get_value64(PID_AC_tauI), get_value64(PID_AC_tauD));
        printf( "PID_BD Params:\n\r");
        printf( "\tKc:%f\ttauI:%f\ttauD:%f\n\r", get_value64(PID_BD_Kc), get_value64(PID_BD_tauI), get_value64(PID_BD_tauD));
#endif

        // Update PID tuning values
        pidAC.SetTunings( get_value64(PID_AC_Kc), get_value64(PID_AC_tauI), get_value64(PID_AC_tauD) );
        pidBD.SetTunings( get_value64(PID_BD_Kc), get_value64(PID_BD_tauI), get_value64(PID_BD_tauD) );

        /* Compute the PID values (this functions returns false and does nothing if set in MANUAL mode) */
        if ( ( pidAC.Compute() == false ) ) {
            // Use the heater values provided by the user
            voutAC = get_value64(HeaterAC);

            // Check if the user set values within the DAC range
            if (get_value64(HeaterAC) > PID_OUTMAX) {
                voutAC = PID_OUTMAX;
            }
            if (get_value64(HeaterAC) < PID_OUTMIN) {
                voutAC = PID_OUTMIN;
            }
        }

        if ( ( pidBD.Compute() == false ) ) {
            // Use the heater values provided by the user
            voutBD = get_value64(HeaterBD);

            // Check if the user set values within the DAC range
            if (get_value64(HeaterBD) > PID_OUTMAX) {
                voutBD = PID_OUTMAX;
            }
            if (get_value64(HeaterBD) < PID_OUTMIN) {
                voutBD = PID_OUTMIN;
            }
        }

        // Update values in BSMP registers list
        set_value(HeaterAC, voutAC);
        set_value(HeaterBD, voutBD);

        AC_Heater_DAC.Write( voutAC );
        BD_Heater_DAC.Write( voutBD );

#ifdef DEBUG_PRINTF
        printf("Heater output AC: %f \t BD: %f\n\r", voutAC, voutBD);
#endif
        Thread::wait(100);
    }
}

void Attenuators_Control( void )
{
    double prev_att1 = 0;
    bool attVec1[6];

    printf("Initializing Attenuators thread\n\r");

    while (1) {
        // Attenuators set
        Thread::signal_wait(0x01);
        if ( prev_att1 != get_value64(Att) ) {
            // Checking and setting attenuators value to fisable values
            set_value(Att,(float)(int(get_value64(Att)*2))/2);
#ifdef DEBUG_PRINTF
            printf("\n\rAtt values updated from: %f to %f\n\r", prev_att1, get_value64(Att));
#endif
	    /* Save new value to FeRAM */
	    feram.set_attenuation(Att[0]);

            // Updating previous values
            prev_att1 = get_value64(Att);
            int2bin6(int(prev_att1*2), attVec1);

            LE = 0;
            clk = 0;
            // Serial data to attenuators
            for (int i = 5; i >= 0; i--) {
                dataA = attVec1[i];
                dataB = attVec1[i];
                dataC = attVec1[i];
                dataD = attVec1[i];
                clk = 0;
                Thread::wait(1);
                clk = 1;
                Thread::wait(1);
            }
            // Falling edge on Latch Enable pin
            LE = 1;
            Thread::wait(1);
            LE = 0;
        }
    }
}

char cli_cmd[SCMD_MAX_CMD_LEN+1];

void commandCallback(char *cmdIn, void *extraContext)
{
    // all our commands will be recieved async in commandCallback
    // we don't want to do time consuming things since it could
    // block the reader and allow the uart to overflow so we simply
    // copy it out in the callback and then process it latter.
    strcpy(cli_cmd, cmdIn);

    CLI_Proccess_Thread.signal_set(0x01);
}

void CLI_Proccess( void )
{
    char *cmd, *save_ptr;
    char *arg[2];
    uint8_t msg_buffer[30] = {0};

    printf("Initializing CLI_Proccess thread\n\r");

    for( ; ; ) {
        Thread::signal_wait(0x01);
        cmd = strtok_r( cli_cmd, " ", &save_ptr);

        for ( uint8_t i = 0; i < sizeof(arg)/sizeof(arg[0]); i++) {
            arg[i] = strtok_r( NULL, " ", &save_ptr);
        }
        printf("\r\n");
        if (strncmp( cmd, "dump", 5 ) == 0) {
            printf("RFFE Vars dump:\n\r");
            printf("\t[0]  Att: %f\n\r", get_value64(Att));
            printf("\t[1]  Temperature AC: %f\n\r", get_value64(TempAC));
            printf("\t[2]  Temperature BD: %f\n\r", get_value64(TempBD));
            printf("\t[3]  Set PointAC: %f\n\r", get_value64(Set_PointAC));
            printf("\t[4]  Set PointBD: %f\n\r", get_value64(Set_PointBD));
            printf("\t[5]  Temperature Control PID: %s\n\r", get_value8(Temp_Control) ? "AUTOMATIC":"MANUAL");
            printf("\t[6]  Heater AC: %f\n\r", get_value64(HeaterAC));
            printf("\t[7]  Heater BD: %f\n\r", get_value64(HeaterBD));
            printf("\t[8]  Reset: %d\n\r", get_value8(Reset));
            printf("\t[9]  Reprogramming: %d\n\r", get_value8(Reprogramming));
            printf("\t[10] New FW Data\n\r");
            printf("\t[11] Firmware version: %s\n\r", FW_VERSION);
            printf("\t[12] PID_AC_Kc: %f\n\r", get_value64(PID_AC_Kc));
            printf("\t[13] PID_AC_tauI: %f\n\r", get_value64(PID_AC_tauI));
            printf("\t[14] PID_AC_tauD: %f\n\r", get_value64(PID_AC_tauD));
            printf("\t[15] PID_BD_Kc: %f\n\r", get_value64(PID_BD_Kc));
            printf("\t[16] PID_BD_tauI: %f\n\r", get_value64(PID_BD_tauI));
            printf("\t[17] PID_BD_tauD: %f\n\r", get_value64(PID_BD_tauD));
            printf("\t[18] IP-Address: %s\n\r", IP_Addr);
            printf("\t[19] MAC-Address: %s\n\r", MAC_Addr);
            printf("\t[20] Gateway-Address: %s\n\r", Gateway_Addr);
            printf("\t[21] Mask-Address: %s\n\r", Mask_Addr);
            printf("\n\r");
        } else if (strncmp( cmd, "set", 4 ) == 0) {
            if ((arg[0] == NULL) || (arg[1] == NULL)) {
                printf("Command \"set\" used but no arguments given! Type \"help\" to see its correct usage.\n\r");
                continue;
            }
            uint8_t var_index = strtol( arg[0], NULL, 10);

            if (rffe_vars[var_index].info.writable == READ_ONLY) {
                printf("The requested variable is READ_ONLY!\n\r");
                continue;
            }

            /* Clear request msg buffer */
            memset(msg_buffer, 0, sizeof(msg_buffer));

            bsmp_mail_t *mail = bsmp_mail_box.alloc();

            mail->response_mail_box = NULL;
            mail->msg.data = msg_buffer;
            mail->msg.data[0] = 0x20; /* CMD_VAR_WRITE */
            mail->msg.data[3] = var_index; /* Payload[0]: Var id */

            uint16_t payload_size = 0;

            switch(rffe_vars[var_index].info.size) {
            case sizeof(int):
            {
                int arg_int = strtol( arg[1], NULL, 10);
                memcpy(&(mail->msg.data[4]), &arg_int, sizeof(arg_int));
                payload_size = sizeof(int);
                break;
            }
            case sizeof(double):
            {
                double arg_dbl = strtod( arg[1], NULL);
                memcpy(&(mail->msg.data[4]), &arg_dbl, sizeof(arg_dbl));
                payload_size = sizeof(double);
                break;
            }
            case sizeof(uint8_t):
            {
                uint8_t arg_byte = strtoul( arg[1], NULL, 10);
                memcpy(&(mail->msg.data[4]), &arg_byte, sizeof(arg_byte));
                payload_size = sizeof(uint8_t);
                break;
            }
            default:
                strncpy((char *)&(mail->msg.data[4]), arg[1], rffe_vars[var_index].info.size);
                payload_size = rffe_vars[var_index].info.size;
            }

            payload_size++; /* Var id counts as payload */
            mail->msg.data[1] = (payload_size >> 8) & 0xFF; /* Payload size >> 8 */
            mail->msg.data[2] = payload_size & 0xFF; /* Payload size */
            mail->msg.len = payload_size + 3;

            bsmp_mail_box.put(mail);

        } else if ((strncmp( cmd, "help", 5 ) == 0) || (strncmp( cmd, "?", 2 ) == 0) ) {
            printf("RFFE Firmware help. Available commands:\n\r");
            printf("\tCMD\t[arg1]\t[arg2]\n\r");
            printf("\tdump\t\t\tList all variables available and their current status\n\r");
            printf("\tset\t[VAR]\t[VALUE]\tSet value to a variable in the list\n\r");
            printf("\thelp\t\t\tShow this help menu\n\r");
        } else {
            printf("Command \"%s\" not recognized! Please use the command \"help\" to check the CLI usage\n\r", cli_cmd);
        }
    }
}

void bsmp_hook_signal_threads(enum bsmp_operation op, struct bsmp_var **list)
{
    bsmp_var *var = NULL;
    uint8_t i = 0;

    if (op == BSMP_OP_READ) return;

    for (i = 0; list[i] != NULL; i++) {
        var = list[i];
        /* Special cases */
        switch( var->info.id ) {
        case 0:
            /* Attenuators */
            Attenuators_thread.signal_set(0x01);
            break;
	case 1:
	case 2:
	case 6:
	case 7:
	    wdt_heartbeat();
	    break;
        case 8:
            /* Reset */
            printf("Resetting MBED...\n\r");
            mbed_reset();
            break;
        case 18:
            /* IP Address */
            printf("Updating IP address on FeRAM to %s ...\n\r", IP_Addr);
            feram.set_ip_addr(IP_Addr);
            break;
        case 19:
            /* MAC Address */
            printf("Updating MAC address on FeRAM to %s ...\n\r", MAC_Addr);
            feram.set_mac_addr(MAC_Addr);
            break;
        case 20:
            /* Gateway Address */
            printf("Updating Gateway address on FeRAM to %s ...\n\r", Gateway_Addr);
            feram.set_gateway_addr(Gateway_Addr);
            break;
        case 21:
            /* Mask Address */
            printf("Updating Mask address on FeRAM to %s ...\n\r", Mask_Addr);
            feram.set_mask_addr(Mask_Addr);
            break;
        }
    }
}

void bsmp_dispatcher( void )
{
    struct bsmp_raw_packet mock_response;

    while(1) {
        /*  Wait for a new message */
        osEvent evt = bsmp_mail_box.get();

        if (evt.status != osEventMail) {
            /* Quietly ignore errors for now */
            continue;
        }

        bsmp_mail_t *mail = (bsmp_mail_t*)evt.value.p;

        mock_response.data = (uint8_t *) malloc(sizeof(uint8_t)*30);

        /* Proccess BSMP request */
        bsmp_process_packet(bsmp, &mail->msg, &mock_response);

        /* Only respond if there's a valid mailbox to put the response in */
        if (mail->response_mail_box) {
            struct bsmp_raw_packet *response = mail->response_mail_box->alloc();
            response->data = mock_response.data;
            response->len = mock_response.len;
            mail->response_mail_box->put(response);
        } else {
	    free(mock_response.data);
	}

        bsmp_mail_box.free(mail);
    }
}

int main( void )
{
    wdt.clear_overflow_flag();

    //Init serial port for info printf
    pc.baud(115200);

    printf("Starting RFFEuC firmware "FW_VERSION" !\n\n\r");

    bsmp = bsmp_server_new();

    bsmp_register_hook(bsmp, bsmp_hook_signal_threads);

    led_g=0;
    led_r=0;

    // Variables initialization
    // Attenuators
    //set_value(Att, 30.0);
    // TempAC
    set_value(TempAC, 0.0);
    // TempBD
    set_value(TempBD, 0.0);
    // Set_PointAC
    set_value(Set_PointAC, 51.5);
    // Set_PointBD
    set_value(Set_PointBD, 51.5);
    // Temp_Control
    set_value(Temp_Control, 0);
    // HeaterAC
    set_value(HeaterAC, 0.0);
    // HeaterBD
    set_value(HeaterBD, 0.0);
    // Reset
    set_value(Reset, 0);
    // Reprogramming
    set_value(Reprogramming, 0);
    // Version
    set_value(Version, FW_VERSION, sizeof(FW_VERSION));
    //PID_AC Kc parameter
    set_value(PID_AC_Kc, 10.5);
    //PID_AC tauI parameter
    set_value(PID_AC_tauI, 12);
    //PID_AC tauI parameter
    set_value(PID_AC_tauD, 2);
    //PID_BD Kc parameter
    set_value(PID_BD_Kc, 10.5);
    //PID_BD tauI parameter
    set_value(PID_BD_tauI, 12);
    //PID_BD tauI parameter
    set_value(PID_BD_tauD, 2);

    feram.get_attenuation(Att);
    feram.get_ip_addr(IP_Addr);
    feram.get_gateway_addr(Gateway_Addr);
    feram.get_mask_addr(Mask_Addr);
    feram.get_mac_addr(MAC_Addr, mac_buffer);
    printf("Ethernet configuration from FeRAM:\n\r");
    printf("\tIP : %s\n\r", IP_Addr);
    printf("\tMask : %s\n\r", Mask_Addr);
    printf("\tGateway : %s\n\r", Gateway_Addr);
    printf("\tMAC : %s\n\r", MAC_Addr);
    printf("\n\r");

    for ( uint8_t i = 0; i < sizeof(rffe_vars)/sizeof(rffe_vars[0]); i++) {
        rffe_vars[i].info.id = i;
        bsmp_register_variable( bsmp, &rffe_vars[i] );
    }

    // *************************************Threads***************************************
    Attenuators_thread.start(Attenuators_Control);
    Temp_Control_thread.start(Temp_Feedback_Control);
    CLI_Proccess_Thread.start(CLI_Proccess);
    BSMP_Thread.start(bsmp_dispatcher);

    // Instantiate our command processor for the  USB serial line.
    scMake(&pc, commandCallback, NULL);

    IAP iap;
    uint8_t *fw_buffer;
    uint8_t state = 0;
    uint32_t fw_size_cnt = 0;
    bool full_page = false;
    uint8_t v_major = 0, v_minor = 0, v_patch = 0;

    uint8_t buf[BUFSIZE];

    led4 = !pll.cfg_eth();

    Thread::wait(100);

    // Ethernet initialization
    EthernetInterface net;
    TCPSocket client;
    SocketAddress client_addr;
    TCPServer server;

    int recv_sz, sent_sz;

#if defined(ETH_DHCP)
    net.set_dhcp(true);
#else
#if defined(ETH_FIXIP)
    net.set_network(IP_Addr,Mask_Addr,Gateway_Addr);
#else
#error "No Ethernet addressing mode selected! Please choose between DHCP or Fixed IP!"
#endif
#endif

    while (true) {
        printf("Trying to bring up ethernet connection...\n\r");
        while (net.connect() != 0) {
            printf("Attempt failed. Trying again in 0.5s... \n\r");
            Thread::wait(500);
        }
        printf("Success! RFFE eth server is up!\n\r");

        printf("RFFE IP: %s\n\r", IP_Addr);
        printf("RFFE MAC Address: %s\n\r", MAC_Addr);

        printf("Listening on port %d...\n\r", SERVER_PORT);

        server.open(&net);
        server.bind(net.get_ip_address(), SERVER_PORT);
        server.listen();

        while (true) {
            printf("Waiting for new client connection...\n\r");

            server.accept(&client, &client_addr);
            client.set_blocking(1500);

            printf("Connection from client: %s\n\r", client_addr.get_ip_address());

            while ( get_eth_link_status() ) {

                /* Wait to receive data from client */
                recv_sz = client.recv((char*)buf, 3);

                if (recv_sz == 3) {
                    /* We received a complete message header */
                    uint16_t payload_len = (buf[1] << 8) | buf[2];
                    /* Check if we need to receive some more bytes. This
                     * fixes #9 github issue, in that we end up stuck here
                     * waiting for more bytes that never comes */
                    if (payload_len > 0) {
                        recv_sz += client.recv( (char*) &buf[3], payload_len );
                    }
                } else if (recv_sz <= 0) {
                    /* Special case for disconnections - just discard the socket and await a new connection */
                    break;
                } else {
                    printf("Received malformed message header of size: %d , discarding...", recv_sz );
                    continue;
                }

#ifdef DEBUG_PRINTF
                printf("Received message of %d bytes: ", recv_sz);
                for (int i = 0; i < recv_sz; i++) {
                    printf("0x%X ",buf[i]);
                }
                printf("\n\r");
#endif
                bsmp_mail_t *mail = bsmp_mail_box.alloc();

                mail->response_mail_box = &eth_mail_box;
                mail->msg.data = buf;
                mail->msg.len = recv_sz;

                bsmp_mail_box.put(mail);

                osEvent evt = eth_mail_box.get();

                if (evt.status != osEventMail) {
                    /* Quietly ignore errors for now */
                    continue;
                }

                struct bsmp_raw_packet *response_mail = (struct bsmp_raw_packet *)evt.value.p;

                sent_sz = client.send((char*)response_mail->data, response_mail->len);

#ifdef DEBUG_PRINTF
                printf("Sending message of %d bytes: ", sent_sz);
                for (int i = 0; i < sent_sz; i++) {
                    printf("0x%X ",response_mail->data[i]);
                }
                printf("\n\r");
#endif
                free(response_mail->data);
                eth_mail_box.free(response_mail);

                if (sent_sz <= 0) {
                    printf("ERROR while writing to socket!\n\r");
                    continue;
                }

                if (state != get_value8(Reprogramming)) {
                    switch (get_value8(Reprogramming)) {
                    case 1:
                        /* Read new firmware version */
                        v_major = Data[0];
                        v_minor = Data[1];
                        v_patch = Data[2];
                        fw_size_cnt = 0;
                        fw_buffer = (uint8_t *) malloc(256);
                        __disable_irq();
                        iap.erase( UPDATE_SECTOR_START, UPDATE_SECTOR_END);
                        __enable_irq();
                        break;

                    case 2:
                    {
                        uint32_t *last_page_addr = (uint32_t *)(UPDATE_ADDRESS_OFFSET+0x37f00);
                        memcpy(fw_buffer, last_page_addr, 256);
                        /* Write the bootloader magic word in the last 4 bytes of the page */
                        const uint32_t magic_word[] = {BOOTLOADER_MAGIC_WORD};
                        memcpy(&fw_buffer[252], magic_word, sizeof(magic_word));

                        /* Store version number */
                        fw_buffer[248] = v_major;
                        fw_buffer[249] = v_minor;
                        fw_buffer[250] = v_patch;

                        __disable_irq();
                        iap.write((uint32_t *)&fw_buffer[0], last_page_addr, 256);
                        __enable_irq();
                        free(fw_buffer);
                        break;
                    }
                    default:
                        break;
                    }
                    state = get_value8(Reprogramming);
                }

                if (get_value8(Reprogramming) == 1 && buf[0] == 0x20 && buf[3] == 0x0A ) {
                    if (full_page == false) {
                        memcpy(fw_buffer, Data, FILE_DATASIZE);
                        full_page = true;
                    } else {
                        memcpy(&fw_buffer[FILE_DATASIZE], Data, FILE_DATASIZE);
                        /* A full firmware page was sent, copy data to file */
                        __disable_irq();
                        iap.write((uint32_t *)&fw_buffer[0], (uint32_t *)(UPDATE_ADDRESS_OFFSET+fw_size_cnt), 256);
                        __enable_irq();
                        fw_size_cnt += FILE_DATASIZE*2;
                        full_page = false;
                    }
                }

                if (get_value8(Reset) == 1) {
                    printf("Resetting MBED!\n\r");
                    mbed_reset();
                }
            }

            client.close();
            printf("Client Disconnected!\n\r");

            if (get_eth_link_status() == 0) {
                /* Eth link is down, clean-up server connection */
                server.close();
                net.disconnect();
                break;
            }
        }
    }
}
