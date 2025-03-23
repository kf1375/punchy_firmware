import sys
import re
from PySide6.QtWidgets import QApplication, QWidget, QVBoxLayout, QPushButton, QLabel, QTextEdit, QComboBox, QDial
from PySide6.QtSerialPort import QSerialPort, QSerialPortInfo
from PySide6.QtCore import QIODevice

class MotorSimulator(QWidget):
    def __init__(self):
        super().__init__()

        self.serialPort = QSerialPort()
        self.motorPosition = 0

        self.initUI()
        self.refreshSerialPorts()

        self.serialPort.readyRead.connect(self.read)

    def initUI(self):
        self.setWindowTitle("Motor Controller")
        self.setGeometry(100, 100, 400, 400)
        layout = QVBoxLayout()

        # Serial Port Selector
        self.portSelector = QComboBox(self)
        layout.addWidget(self.portSelector)

        # Connect Button
        self.btnConnect = QPushButton("Connect", self)
        self.btnConnect.clicked.connect(self.connect)
        layout.addWidget(self.btnConnect)

        # Display Motor State
        self.labelState = QLabel("State: Idle", self)
        layout.addWidget(self.labelState)

        # Motor Dial (shows motor position)
        self.dialMotor = QDial(self)
        self.dialMotor.setRange(0, 360)  # Assuming 0 to 360 degrees
        self.dialMotor.setNotchesVisible(True)
        self.dialMotor.setWrapping(True)  # Allows circular rotation
        self.dialMotor.setEnabled(False)  # Read-only
        layout.addWidget(self.dialMotor)

        # Log Output
        self.logOutput = QTextEdit(self)
        self.logOutput.setReadOnly(True)
        layout.addWidget(self.logOutput)

        self.setLayout(layout)

    def refreshSerialPorts(self):
        """Populate the serial port selector."""
        ports = QSerialPortInfo.availablePorts()
        self.portSelector.clear()
        for port in ports:
            self.portSelector.addItem(port.portName())

    def connect(self):
        """Connect to the selected serial port."""
        selectedPort = self.portSelector.currentText()
        if not selectedPort:
            self.log("No port selected.")
            return

        if self.serialPort.isOpen():
            self.serialPort.close()

        self.serialPort.setPortName(selectedPort)
        self.serialPort.setBaudRate(115200)

        if self.serialPort.open(QIODevice.OpenModeFlag.ReadWrite):
            self.log("Connected to " + selectedPort)
        else:
            self.log("Connection failed!")

    def read(self):
        """Read incoming state updates and motor position from ESP32."""
        if self.serialPort and self.serial_port.isOpen():
            while self.serialPort.bytesAvailable() > 0:
                line = self.serialPort.readline().decode("utf-8").strip()
                if line.startswith("STATE:"):
                    state = line.split("STATE:")[1]
                    self.label_state.setText(f"State: {state}")
                elif line.startswith("POSITION:"):
                    position = int(line.split("POSITION:")[1])
                    self.motorPosition = position % 360  # Keep within 0-360
                    self.dialMotor.setValue(self.motorPosition)

                self.log(f"{line}")

    def log(self, message):
        """Display logs in the text box."""
        self.logOutput.append(message)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MotorSimulator()
    window.show()
    sys.exit(app.exec())