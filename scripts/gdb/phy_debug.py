import gdb

def app_read_phy_reg(addr):
    return gdb.parse_and_eval("(uint16_t)lpc17_40_phyread(1, " + str(addr) +")")

def phy_read_BMCR():
    print("Basic Mode Control Register (BMCR):")
    addr = 0x00
    val = app_read_phy_reg(addr)
    print("  RESET: " + str(int((val & 1 << 15) != 0)))
    print("  LOOPBACK: " + str(int((val & 1 << 14) != 0)))
    print("  SPEED_SELECTION: " + str(int((val & 1 << 13) != 0)))
    print("  AUTO_NEGOTIATION_ENABLE: " + str(int((val & 1 << 12) != 0)))
    print("  POWER_DOWN: " + str(int((val & 1 << 11) != 0)))
    print("  ISOLATE: " + str(int((val & 1 << 10) != 0)))
    print("  RESTART_AUTO_NEGOTIATION: " + str(int((val & 1 << 9) != 0)))
    print("  DUPLEX_MODE: " + str(int((val & 1 << 8) != 0)))
    print("  COLLISION_TEST: " + str(int((val & 1 << 7) != 0)))

def phy_read_BMSR():
    print("Basic Mode Status Register (BMSR):")
    addr = 0x01
    val = app_read_phy_reg(addr)
    print("  100BASE_T4: " + str(int((val & 1 << 15) != 0)))
    print("  100BASE_TX_FULL_DUPLEX: " + str(int((val & 1 << 14) != 0)))
    print("  100BASE_TX_HALF_DUPLEX: " + str(int((val & 1 << 13) != 0)))
    print("  10BASE_T_FULL_DUPLEX: " + str(int((val & 1 << 12) != 0)))
    print("  10BASE_T_HALF_DUPLEX: " + str(int((val & 1 << 11) != 0)))
    print("  MF_PREAMBLE_SUPPRESSION: " + str(int((val & 1 << 6) != 0)))
    print("  AUTO_NEGOTIATION_COMPLETE: " + str(int((val & 1 << 5) != 0)))
    print("  REMOTE_FAULT: " + str(int((val & 1 << 4) != 0)))
    print("  AUTO_NEGOTIATION_ABILITY: " + str(int((val & 1 << 3) != 0)))
    print("  LINK_STATUS: " + str(int((val & 1 << 2) != 0)))
    print("  JABBER_DETECT: " + str(int((val & 1 << 1) != 0)))
    print("  EXTENDED_CAPABILITY: " + str(int((val & 1 << 0) != 0)))

def phy_read_PHYIDR1():
    addr = 0x02
    val = app_read_phy_reg(addr)

def phy_read_PHYIDR2():
    addr = 0x03
    val = app_read_phy_reg(addr)

def phy_read_ANAR():
    addr = 0x04
    val = app_read_phy_reg(addr)

def phy_read_ANLPAR():
    addr = 0x04
    val = app_read_phy_reg(addr)

def phy_read_ANLPARNP():
    addr = 0x05
    val = app_read_phy_reg(addr)

def phy_read_ANER():
    addr = 0x06
    val = app_read_phy_reg(addr)

def phy_read_ANNPTR():
    addr = 0x07
    val = app_read_phy_reg(addr)

def phy_read_PHYSTS():
    print("PHY Status Register (PHYSTS):")
    addr = 0x10
    val = app_read_phy_reg(addr)
    print("  MDI_X_MODE: " + str(int((val & 1 << 14) != 0)))
    print("  RECEIVE_ERROR_LATCH: " + str(int((val & 1 << 13) != 0)))
    print("  POLARITY_STATUS: " + str(int((val & 1 << 12) != 0)))
    print("  FALSE_CARRIER_SENSE_LATCH: " + str(int((val & 1 << 11) != 0)))
    print("  SIGNAL_DETECT: " + str(int((val & 1 << 10) != 0)))
    print("  DESCRAMBLER_LOCK: " + str(int((val & 1 << 9) != 0)))
    print("  PAGE_RECEIVED: " + str(int((val & 1 << 8) != 0)))
    print("  REMOTE_FAULT: " + str(int((val & 1 << 6) != 0)))
    print("  JABBER_DETECT: " + str(int((val & 1 << 5) != 0)))
    print("  AUTO_NEG_COMPLETE: " + str(int((val & 1 << 4) != 0)))
    print("  LOOPBACK_STATUS: " + str(int((val & 1 << 3) != 0)))
    print("  DUPLEX_STATUS: " + str(int((val & 1 << 2) != 0)))
    print("  SPEED_STATUS: " + str(int((val & 1 << 1) != 0)))
    print("  LINK_STATUS: " + str(int((val & 1 << 0) != 0)))

def phy_read_FCSCR():
    addr = 0x14
    val = app_read_phy_reg(addr)

def phy_read_RECR():
    addr = 0x15
    val = app_read_phy_reg(addr)

def phy_read_PCSR():
    addr = 0x16
    val = app_read_phy_reg(addr)

def phy_read_RBR():
    print("RMII and Bypass Register (RBR):")
    addr = 0x17
    val = app_read_phy_reg(addr)
    print("  RMII_MOD: " + str(int((val & 1 << 5) != 0)))
    print("  RMII_REV1_0: " + str(int((val & 1 << 4) != 0)))
    print("  RX_OVF_STS: " + str(int((val & 1 << 3) != 0)))
    print("  RX_UNF_STS: " + str(int((val & 1 << 2) != 0)))
    print("  ELAST_BUF: " + str(int((val & 0x03) != 0)))

def phy_read_LEDCR():
    addr = 0x18
    val = app_read_phy_reg(addr)

def phy_read_PHYCR():
    print("PHY Control Register (PHYCR):")
    addr = 0x19
    val = app_read_phy_reg(addr)
    print(" MDIX_EN: " + str(int((val & 1 << 15) != 0)))
    print(" FORCE_MDIX: " + str(int((val & 1 << 14) != 0)))
    print(" PAUSE_RX: " + str(int((val & 1 << 13) != 0)))
    print(" PAUSE_TX: " + str(int((val & 1 << 12) != 0)))
    print(" BIST_FE: " + str(int((val & 1 << 11) != 0)))
    print(" PSR_15: " + str(int((val & 1 << 10) != 0)))
    print(" BIST_STATUS: " + str(int((val & 1 << 9) != 0)))
    print(" BIST_START: " + str(int((val & 1 << 8) != 0)))
    print(" BP_STRETCH: " + str(int((val & 1 << 7) != 0)))
    print(" LED_CNFG: " + str(int((val & 1 << 5) != 0)))
    print(" PHYADDR: " + str(int(val & 0x1F)))

def phy_read_10BT_SCR():
    addr = 0x1A
    val = app_read_phy_reg(addr)

def phy_read_CDCTRL1():
    addr = 0x1B
    val = app_read_phy_reg(addr)

def phy_read_EDCR():
    addr = 0x1C
    val = app_read_phy_reg(addr)

phy_reg_dict = {
    "BMCR" : phy_read_BMCR,
    "BMSR" : phy_read_BMSR,
    "PHYIDR1" : phy_read_PHYIDR1,
    "PHYIDR2" : phy_read_PHYIDR2,
    "ANAR" : phy_read_ANAR,
    "ANLPAR" : phy_read_ANLPAR,
    "ANLPARNP" : phy_read_ANLPARNP,
    "ANER" : phy_read_ANER,
    "ANNPTR" : phy_read_ANNPTR,
    "PHYSTS" : phy_read_PHYSTS,
    "FCSCR" : phy_read_FCSCR,
    "RECR" : phy_read_RECR,
    "PCSR" : phy_read_PCSR,
    "RBR" : phy_read_RBR,
    "LEDCR" : phy_read_LEDCR,
    "PHYCR" : phy_read_PHYCR,
    "10BT_SCR" : phy_read_10BT_SCR,
    "CDCTRL1" : phy_read_CDCTRL1,
    "EDCR" : phy_read_EDCR,
}

class phy_read (gdb.Command):
    """Read a phy register"""
    def __init__ (self):
        super (phy_read, self).__init__ ("phy_read", gdb.COMMAND_USER)

    def invoke (self, args, from_tty):
        argv = gdb.string_to_argv (args)
        if len (argv) != 1:
            raise gdb.GdbError ("phy_read expects the register name as a single argument")
        try:
            callback = phy_reg_dict[argv[0]]
            callback()
        except:
            raise gdb.GdbError ("Register '" + argv[0] + "' unknown")
