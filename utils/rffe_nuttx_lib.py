# -*- coding: utf-8 -*-

import socket
import re

class RFFEFWUpdate:
    def __init__(self, ip_addr, port = 9090):
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.s.connect((ip_addr, port))

    def erase_all(self):
        self.s.send(b"e")
        ans = self.s.recv(1)

    def write_sector(self, data, start_addr):
        """Write data to flash. The data should be a 256 bytes array, and the start address should be 256 bytes aligned"""
        if len(data) != 256:
            raise Exception("data should be a 256 bytes array")

        if ((start_addr % 256) != 0):
            raise Exception("start_addr should be a 256 bytes aligned")

        addr = bytearray.fromhex("FF FF FF FF")
        self.s.send(b"w")
        addr[0] = start_addr & 0xFF
        addr[1] = (start_addr >> 8) & 0xFF
        addr[2] = (start_addr >> 16) & 0xFF
        addr[3] = (start_addr >> 24) & 0xFF
        self.s.send(addr)
        self.s.send(data)
        ans = self.s.recv(1)

    def reset(self):
        self.s.send(b"r")
        self.s.close()

    def close(self):
        self.s.close()

class RFFEControllerBoard:
    """Class used to send commands and acquire data from the RF front-end controller board."""
    def __init__(self, ip, port=9001):
        """Class constructor. Here the socket connection to the board is initialized. The argument
        required is the IP adress of the instrument (string)."""
        self.ip = ip
        self.port = port

        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        self.sock.settimeout(5.0)
        self.sock.connect((self.ip,self.port))

    def __sock_send_line__(self, line):
        line = line + "\n"
        self.sock.send(line.encode("UTF-8"))

    def __sock_recv_line__(self):
        buf = bytearray()
        while True:
            data = self.sock.recv(1)
            if data != None:
                buf.extend(data)
                if buf[-1] == 0x0A:
                    break
        return buf.decode("UTF-8")

    def __scpi_request__(self, req):
        """SCPI request method"""
        ans = ""
        req = req.strip()
        self.__sock_send_line__(req)
        if req[-1] == "?":
            ans = self.__sock_recv_line__()
        return ans

    def get_attenuator_value(self):
        """This method returns the current attenuation value (in dB) as a floating-point number.
           The attenuation value will be between 0 dB and 31.5 dB, with a 0.5 dB step size."""
        return float(self.__scpi_request__("GET:ATTEnuation?"))

    def set_attenuator_value(self, value):
        """Sets the attenuation value of both front-ends. The agrument should be a
        floating-point number representing the attenuation (in dB) between 0 dB and 31.5 dB, with a
        0.5 dB step size."""
        self.__scpi_request__("SET:ATTEnuation {}".format(value))

    def get_temp_ac(self):
        """This method returns the temperature measured by the sensor present in the A/C
        front-end. The value returned is a floating-point number."""
        return float(self.__scpi_request__("MEASure:TEMPerature:AC?"))

    def get_temp_bd(self):
        """This method returns the temperature measured by the sensor present in the B/D
        front-end. The value returned is a floating-point number."""
        return float(self.__scpi_request__("MEASure:TEMPerature:BD?"))

    def get_temp_ac_setpoint(self):
        """This method returns the temperature set-point for the A/C front-end temperature
        controller. The returned value is a floating-point number in the Celsius degrees scale."""
        return float(self.__scpi_request__("GET:TEMPerature:SETPoint:AC?"))

    def set_temp_ac_setpoint(self, value):
        """Sets the temperature set-point for the A/C front-end temperature controller. The value
        passed as the argument is a floating-point number."""
        self.__scpi_request__("SET:TEMPerature:SETPoint:AC {}".format(value))

    def get_temp_bd_setpoint(self):
        """This method returns the temperature set-point for the B/D front-end temperature
        controller. The returned value is a floating-point number in the Celsius degrees scale."""
        return float(self.__scpi_request__("GET:TEMPerature:SETPoint:AC?"))

    def set_temp_bd_setpoint(self, value):
        """Sets the temperature set-point for the B/D front-end temperature controller. The value
        passed as the argument is a floating-point number."""
        self.__scpi_request__("SET:TEMPerature:SETPoint:BD {}".format(value))

    def get_temperature_control_status(self):
        """This method returns the temperature controller status as an integer. If this integer
        equals 0, it's because the temperature controller is off. Otherwise, if the value returned
        equals 1, this means the temperature controller is on."""
        return int(self.__scpi_request__("GET:TEMPControl:AUTOmatic?"))

    def set_temperature_control_status(self, status):
        """Method used to turn on/off the temperature controller. For turning the controller on, the
        argument should be the integer 1. To turn the controller off, the argument should be 0."""
        if (status in (0, 1)):
            self.__scpi_request__("SET:TEMPControl:AUTOmatic {}".format(status))

    def get_heater_ac_value(self):
        """This method returns the voltage signal to the heater in the A/C front-end as a
        floating-point number."""
        return float(self.__scpi_request__("GET:DAC:OUTput:AC?"))

    def set_heater_ac_value(self, value):
        """Sets the voltage level to the heater in the A/C front-end. The value passed as the
        argument, a floating-point number, is the intended voltage for the heater."""
        self.__scpi_request__("SET:DAC:OUTput:AC {}".format(value))

    def get_heater_bd_value(self):
        """This method returns the voltage signal to the heater in the B/D front-end as a
        floating-point number."""
        return float(self.__scpi_request__("GET:DAC:OUTput:BD?"))

    def set_heater_bd_value(self, value):
        """Sets the voltage level to the heater in the B/D front-end. The value passed as the
        argument, a floating-point number, is the intended voltage for the heater."""
        self.__scpi_request__("SET:DAC:OUTput:BD {}".format(value))

    def reset(self):
        """This method resets the board software."""
        self.__scpi_request__("SYSTem:RESet")

    def reprogram(self, file_path, version, bootloader=False):
        """This method reprograms the mbed device on the RF front-end controller board. The
        first argument, a string, is the path to the binary file which corresponds to the
        program will be loaded in the device. The second argument is the new firmware version
        formated as: x.y.z or x_y_z"""
        pass
        major, minor, patch = map(int,re.split('[., _]',version))

        start_addr = 0x48000

        rffe_fw = RFFEFWUpdate(self.ip)

        with open(file_path, "rb") as f:

            rffe_fw.erase_all()

            buf = f.read(256)
            while buf != b"":

                pad = bytearray()
                pad.extend(buf)

                if len(buf) < 256:
                    pad.extend(b'\377' * (256 - len(pad)))

                rffe_fw.write_sector(pad, start_addr)
                start_addr += 256
                buf = f.read(256)

        boot_sec = bytearray()
        boot_sec.extend(b'\377' * 256)

        boot_sec[248] = major
        boot_sec[249] = minor
        boot_sec[250] = patch
        boot_sec[251] = 2 if bootloader else 1
        boot_sec[252] = 0xAA
        boot_sec[253] = 0xAA
        boot_sec[254] = 0xAA
        boot_sec[255] = 0xAA
        rffe_fw.write_sector(boot_sec, 0x0007FF00)
        rffe_fw.reset()
        self.close()

    def set_pid_ac_kc(self, value):
        """Sets the PID Kc parameter in the A/C front-end. The value is passed as a floating-point numpber."""
        self.__scpi_request__("SET:PID:Kc:AC {}".format(value))

    def get_pid_ac_kc(self):
        """This method returns the Kc parameter of the PID in the A/C front-end as a
        floating-point number."""
        return float(self.__scpi_request__("GET:PID:Kc:AC?"))

    def set_pid_ac_taui(self, value):
        """Sets the PID tauI parameter in the A/C front-end. The value is passed as a floating-point number."""
        self.__scpi_request__("SET:PID:Ti:AC {}".format(value))

    def get_pid_ac_taui(self):
        """This method returns the tauI parameter of the PID in the A/C front-end as a
        floating-point number."""
        return float(self.__scpi_request__("GET:PID:Ti:AC?"))

    def set_pid_ac_taud(self, value):
        """Sets the PID tauD parameter in the A/C front-end. The value is passed as a floating-point number."""
        self.__scpi_request__("SET:PID:Td:AC {}".format(value))

    def get_pid_ac_taud(self):
        """This method returns the tauD parameter of the PID in the A/C front-end as a
        floating-point number."""
        return float(self.__scpi_request__("GET:PID:Td:AC?"))

    def set_pid_bd_kc(self, value):
        """Sets the PID Kc parameter in the B/D front-end. The value is passed as a floating-point number."""
        self.__scpi_request__("SET:PID:Kc:BD {}".format(value))

    def get_pid_bd_kc(self):
        """This method returns the Kc parameter of the PID in the B/D front-end as a
        floating-point number."""
        return float(self.__scpi_request__("GET:PID:Kc:BD?"))

    def set_pid_bd_taui(self, value):
        """Sets the PID tauI parameter in the B/D front-end. The value is passed as a floating-point number."""
        self.__scpi_request__("SET:PID:Ti:BD {}".format(value))

    def get_pid_bd_taui(self):
        """This method returns the tauI parameter of the PID in the B/D front-end as a
        floating-point number."""
        return float(self.__scpi_request__("GET:PID:Ti:BD?"))

    def set_pid_bd_taud(self, value):
        """Sets the PID tauD parameter in the B/D front-end. The value is passed as a floating-point number."""
        self.__scpi_request__("SET:PID:Td:BD {}".format(value))

    def get_pid_bd_taud(self):
        """This method returns the tauD parameter of the PID in the B/D front-end as a
        floating-point number."""
        return float(self.__scpi_request__("GET:PID:Td:BD?"))

    def get_mac_address(self):
        """This method returns the MBED MAC_Address as a string"""
        pass

    def set_ip(self, ip):
        """Sets the IP Address. The value is passed as a string."""
        self.__scpi_request__("SET:IPAddr " + ip)

    def close(self):
        self.sock.close()
