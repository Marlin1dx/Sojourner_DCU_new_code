import sys
import serial
import serial.tools.list_ports
import threading
import time
from PyQt5.QtWidgets import (QApplication, QMainWindow, QVBoxLayout, QHBoxLayout, 
                             QLabel, QPushButton, QComboBox, QTextEdit, 
                             QWidget, QGridLayout, QSlider)
from PyQt5.QtCore import Qt, QTimer
from PyQt5.QtGui import QKeyEvent, QFont

class RobotControlPanel(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Sojourner DCU - Панель управления")
        self.setGeometry(100, 100, 1000, 700)
        self.setFocusPolicy(Qt.StrongFocus)

        # Центральный виджет и основной layout
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        main_layout = QHBoxLayout()
        central_widget.setLayout(main_layout)

        # Левая панель управления
        control_widget = QWidget()
        control_layout = QVBoxLayout()
        control_widget.setLayout(control_layout)

        # Создание кнопок управления движением
        movement_layout = QGridLayout()
        
        # Стрелки
        self.btn_forward = QPushButton("↑ Вперед")
        self.btn_backward = QPushButton("↓ Назад")
        self.btn_left = QPushButton("← Влево")
        self.btn_right = QPushButton("→ Вправо")
        
        # Кнопка остановки
        self.btn_stop = QPushButton("■ Стоп")
        self.btn_stop.setStyleSheet("background-color: red; color: white;")
        
        # Настройка скорости
        speed_layout = QHBoxLayout()
        self.speed_label = QLabel("Скорость: 50%")
        self.speed_slider = QSlider(Qt.Horizontal)
        self.speed_slider.setRange(0, 100)
        self.speed_slider.setValue(50)
        self.speed_slider.valueChanged.connect(self.update_speed)
        
        speed_layout.addWidget(QLabel("Скорость"))
        speed_layout.addWidget(self.speed_slider)
        speed_layout.addWidget(self.speed_label)

        # Размещение кнопок
        movement_layout.addWidget(self.btn_forward, 0, 1)
        movement_layout.addWidget(self.btn_left, 1, 0)
        movement_layout.addWidget(self.btn_stop, 1, 1)
        movement_layout.addWidget(self.btn_right, 1, 2)
        movement_layout.addWidget(self.btn_backward, 2, 1)

        # Подключение обработчиков
        self.btn_forward.pressed.connect(lambda: self.send_movement_command('forward'))
        self.btn_backward.pressed.connect(lambda: self.send_movement_command('backward'))
        self.btn_left.pressed.connect(lambda: self.send_movement_command('left'))
        self.btn_right.pressed.connect(lambda: self.send_movement_command('right'))
        self.btn_stop.pressed.connect(lambda: self.send_movement_command('stop'))

        # Порт и кнопка подключения
        port_layout = QHBoxLayout()
        self.port_combo = QComboBox()
        self.refresh_ports_button = QPushButton("Обновить порты")
        self.connect_button = QPushButton("Подключить")
        
        port_layout.addWidget(self.port_combo)
        port_layout.addWidget(self.refresh_ports_button)
        port_layout.addWidget(self.connect_button)

        # Телеметрия
        telemetry_widget = QWidget()
        telemetry_layout = QGridLayout()
        telemetry_widget.setLayout(telemetry_layout)

        telemetry_labels = [
            "Гироскоп X", "Гироскоп Y", "Гироскоп Z",
            "Акселерометр X", "Акселерометр Y", "Акселерометр Z",
            "Магнитометр X", "Магнитометр Y", "Магнитометр Z",
            "Roll", "Pitch", "Yaw",
            "Широта", "Долгота", "Высота",
            "Угол ALF", "Угол ALR", "Угол ARF", "Угол ARR"
        ]

        self.telemetry_values = {}
        for i, label_text in enumerate(telemetry_labels):
            label = QLabel(label_text)
            value_label = QLabel("N/A")
            value_label.setFont(QFont("Monospace", 9))
            telemetry_layout.addWidget(label, i, 0)
            telemetry_layout.addWidget(value_label, i, 1)
            self.telemetry_values[label_text] = value_label

        # Консоль
        self.console = QTextEdit()
        self.console.setReadOnly(True)

        # Компоновка
        control_layout.addLayout(movement_layout)
        control_layout.addLayout(speed_layout)
        control_layout.addLayout(port_layout)

        main_layout.addWidget(control_widget)
        main_layout.addWidget(telemetry_widget)
        main_layout.addWidget(self.console)

        # Настройка портов
        self.serial_port = None
        self.current_speed = 50
        self.refresh_ports()
        self.refresh_ports_button.clicked.connect(self.refresh_ports)
        self.connect_button.clicked.connect(self.toggle_connection)

        # Таймер для обновления
        self.update_timer = QTimer()
        self.update_timer.timeout.connect(self.request_telemetry)
        self.update_timer.start(500)  # Опрос каждые 500 мс

    def keyPressEvent(self, event: QKeyEvent):
        # Управление с клавиатуры
        if event.key() == Qt.Key_W:
            self.send_movement_command('forward')
        elif event.key() == Qt.Key_S:
            self.send_movement_command('backward')
        elif event.key() == Qt.Key_A:
            self.send_movement_command('left')
        elif event.key() == Qt.Key_D:
            self.send_movement_command('right')
        elif event.key() == Qt.Key_Space:
            self.send_movement_command('stop')

    def update_speed(self, value):
        self.current_speed = value
        self.speed_label.setText(f"Скорость: {value}%")

    def send_movement_command(self, direction):
        if not self.serial_port or not self.serial_port.is_open:
            self.log("Порт не подключен")
            return

        # Преобразуем направление и скорость в команду
        speed = self.current_speed
        
        # Команды для моторов (пример, адаптируйте под вашу кинематику)
        commands = {
            'forward':   f"M0:{speed},M1:{speed},M2:{speed},M3:{speed}",
            'backward':  f"M0:{-speed},M1:{-speed},M2:{-speed},M3:{-speed}",
            'left':     f"M0:{-speed},M1:{speed},M2:{speed},M3:{-speed}",
            'right':    f"M0:{speed},M1:{-speed},M2:{-speed},M3:{speed}",
            'stop':     "M0:0,M1:0,M2:0,M3:0"
        }

        command = commands.get(direction, 'stop')
        
        try:
            self.serial_port.write((command + '\n').encode())
            self.log(f"Команда: {command}")
        except Exception as e:
            self.log(f"Ошибка отправки: {e}")

    def refresh_ports(self):
        self.port_combo.clear()
        ports = [port.device for port in serial.tools.list_ports.comports()]
        self.port_combo.addItems(ports)

    def toggle_connection(self):
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.close()
            self.connect_button.setText("Подключить")
            self.log("Отключено")
        else:
            try:
                port = self.port_combo.currentText()
                self.serial_port = serial.Serial(port, 115200, timeout=1)
                self.connect_button.setText("Отключить")
                self.log(f"Подключено к порту {port}")
                threading.Thread(target=self.read_serial, daemon=True).start()
            except Exception as e:
                self.log(f"Ошибка подключения: {e}")

    def request_telemetry(self):
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.write(b'TELEMETRY\n')

    def read_serial(self):
        while self.serial_port and self.serial_port.is_open:
            try:
                data = self.serial_port.readline().decode().strip()
                if data:
                    self.parse_telemetry(data)
            except Exception as e:
                self.log(f"Ошибка чтения: {e}")
                break

    def parse_telemetry(self, data):
        try:
            imu_data, gps_data, hall_data = data.split('|')
            
            # Парсинг IMU
            imu_values = imu_data.split(':')[1].split(',')
            imu_labels = [
                "Гироскоп X", "Гироскоп Y", "Гироскоп Z",
                "Акселерометр X", "Акселерометр Y", "Акселерометр Z",
                "Магнитометр X", "Магнитометр Y", "Магнитометр Z",
                "Roll", "Pitch", "Yaw", "Температура"
            ]
            
            # Парсинг GPS
            gps_values = gps_data.split(':')[1].split(',')
            gps_labels = [
                "Широта", "Долгота", "Высота", "Скорость", "Курс",
                "Спутники", "Фикс", "Час", "Минута", "Секунда",
                "День", "Месяц", "Год"
            ]
            
            # Парсинг углов Холла
            hall_values = hall_data.split(':')[1].split(',')
            hall_labels = ["Угол ALF", "Угол ALR", "Угол ARF", "Угол ARR"]

            # Обновление значений
            for label, value in zip(imu_labels, imu_values):
                self.telemetry_values[label].setText(value)
            
            for label, value in zip(gps_labels, gps_values):
                self.telemetry_values[label].setText(value)
            
            for label, value in zip(hall_labels, hall_values):
                self.telemetry_values[label].setText(value)

        except Exception as e:
            self.log(f"Ошибка парсинга: {e}")

    def log(self, message):
        self.console.append(message)

def main():
    app = QApplication(sys.argv)
    panel = RobotControlPanel()
    panel.show()
    sys.exit(app.exec_())

if __name__ == "__main__":
    main()