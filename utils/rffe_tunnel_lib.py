import paramiko

class SSHTunnel:
    """
    Establishes an SSH tunnel using Paramiko for forwarding data
    between a local machine and a remote host/port via direct-tcpip.

    Args:
        host (str): SSH hostname to connect to.
        username (str): Username for SSH authentication.
        key_path (str): Path to the SSH private key file.
        remote_host (str): Target host to forward the connection to.
        remote_port (int): Target port on the remote host.
        key_pass (str): Passphrase for the SSH private key.
    """
    def __init__(self, host, username, key_path, remote_host, remote_port, key_pass):
        self.host = host
        self.username = username
        self.key_path = key_path
        self.remote_host = remote_host
        self.remote_port = remote_port
        self.key_pass = key_pass
        self.ssh_client = None
        self.ssh_channel = None

    def connect(self):
        """
        Connects to the SSH server and establishes a channel to the remote host.

        Returns:
            A Paramiko Channel object if the connection is successful, or None on failure.
        """
        try: 
            self.ssh_client = paramiko.SSHClient()
            self.ssh_client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
            self.ssh_client.connect(
                hostname=self.host,
                username=self.username,
                key_filename=self.key_path,
                password=self.key_pass,
                banner_timeout=5000
            )

            transport = self.ssh_client.get_transport()

            self.ssh_channel = transport.open_channel(
                "direct-tcpip",
                (self.remote_host, self.remote_port),
                ('127.0.0.1', 0)
            )
            return self.ssh_channel
        except Exception as e:
            print(f"[ERROR] SSH tunnel connection failed: {e}, on {self.host}")
            self.close()
            return None
            
    def close(self):
        """
        Closes the SSH channel and SSH client cleanly.
        """
        if self.ssh_channel:
            self.ssh_channel.close()
        if self.ssh_client:
            self.ssh_client.close()
