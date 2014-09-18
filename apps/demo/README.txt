----------------------------------------------------------
WICED Application Demo Directory - README
----------------------------------------------------------

This directory contains advanced demonstration applications
that combine various WICED features into a single application

Applications in this directory are listed below.

 Home Appliance Control Application
    This application demonstrates how a simple web page can be used to send
    information to a UART when a button on a webpage is clicked. The application
    mimics a very basic user interface to control a home appliance such as a washing
    machine or dryer.

 Temperature Control & Report Application
     This application measures the temperature of the WICED evaluation
     board and sends temperature measurements to the 'Internet of Things' data
     warehouse http://xively.com/. The app also displays temperature & setpoint
     status on a local webpage.

SmartBridge Demo
     This application demonstrates how to use the Bluetooth SmartBridge API. The app
     scans for remote Bluetooth Smart devices and allows users to select which device
     to connect with. Users interact with the app via a webpage running on a local
     webserver. Once a connection to a remote device is established, services and
     characteristic values for the device are reported in real-time on the webpage
