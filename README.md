> **Disclaimer:**  
> This is an experimental project aiming to bring Windows Hello-style authentication features to Linux.  
> At present, poor integration with GNOME Keyring makes it unsuitable for daily use.

# pam-wewe

WèWè is a PAM module for PIN authentication on Linux systems. It allows users to authenticate using a personal PIN, with optional restrictions based on trusted networks.  
The module configuration is managed via the `/etc/wewe/config.yaml` file.  
Users can further configure their PIN and manage their personal list of trusted networks using the `wewe-ctl` command
