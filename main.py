
# from _typeshed import NoneType

import json5
from kivy_garden.mapview.view import MapMarkerPopup, MarkerMapLayer

# import busio
# import board
# import adafruit_amg88xxp
import paho.mqtt.client as mqtt
from kivy.clock import Clock
from kivy.uix.popup import Popup
from kivymd.uix.behaviors import TouchBehavior
from kivymd.uix.button import MDRaisedButton
from kivymd.app import MDApp
from kivy.lang.builder import Builder
from kivy.uix.screenmanager import ScreenManager, Screen
from kivy.uix.button import Button
from kivymd.uix.button import MDRectangleFlatButton
from kivymd.uix.label import MDLabel
from kivy.uix.label import Label
from kivymd.font_definitions import theme_font_styles
from kivy.core.text import LabelBase
from kivymd.uix.navigationdrawer import NavigationLayout
from kivymd.uix.navigationdrawer import MDNavigationDrawer
from kivymd.uix.button import MDFlatButton
from kivymd.uix.dialog import MDDialog
from kivy_garden.mapview import MapView

from kivy.uix.boxlayout import BoxLayout
# screen_helper = """


# """

# class MenuScreen(Screen):
#     pass


class ConfigScreen(Screen):
    pass


class DemoApp(MDApp):
    global flag
    global mqtt_c
    self_heal_count = 0

    mqtt_c = mqtt.Client()

    def build(self):
        #sm = ScreenManager()
       # sm.add_widget(MenuScreen(name='menu'))
        # sm.add_widget(InstScreen(name='inst'))
        # sm.add_widget(ConfigScreen(name='device'))
        self.root.ids.box.add_widget(
            MDLabel(
                text="Instructions",
                halign="left",
                font_style="H2"
                # theme_text_color=name_theme,
            )
        )
        for name_theme in [

            "1. Switch ON the wifi before switching on the device",
            "2. On upper half of screen you will get different specifications about the devices location",
            "3. On bottom half of screen map will be centered on current devices location will be printed",
        ]:
            self.root.ids.box.add_widget(
                MDLabel(
                    text=name_theme,
                    halign="left",
                    # theme_text_color=name_theme,
                )
            )
        Clock.schedule_interval(self.show_data, 20)
        try:
            mqtt_c.connect("broker.emqx.io", 1883, 60)
            mqtt_c.on_log = self.on_log
            mqtt_c.on_message = self.on_message
            mqtt_c.on_disconnect = self.on_disconnect
            # Create the screen manager

            # # Font
            # LabelBase.register(
            #     name="JetBrainsMono",
            #     fn_regular="jetbrains-mono.regular.ttf")
            # theme_font_styles.append('JetBrainsMono')
            # self.theme_cls.font_styles["JetBrainsMono"] = [
            #     "JetBrainsMono",
            #     16,
            #     True,
            #     0.15,
            # ]
            # Theming
            self.theme_cls.primary_palette = "Cyan"
            self.theme_cls.theme_style = "Light"  # "Dark"
            if not mqtt_c:
                print('Mqtt Not Conection')
                content = Button(text='Close me!')

                flag = 0
            else:
                print('Mqtt Conected')
                flag = 1
                mqtt_c.subscribe('dubblin/location')

                mqtt_count = 0
                mqtt_c.loop_start()

        except:

            # Attach close button press with popup.dismiss action

            popup = Popup(title='No Internet',
                          content=Label(text='Switch on Wifi'),
                          size_hint=(None, None), size=(400, 400))

# bind the on_press event of the button to the dismiss function


# open the popup
            popup.open()

    def on_disconnect(self, client, userdata, message):
        dialog = MDDialog(
            text="Lost Internet Connection",

        )
        dialog.open()

    def ac_learn(self):
        # get a reference to the AddWindow Screen
        addwindow_instance = self.root.ids.screen_manager.get_screen("device")
        m_id = addwindow_instance.ids["mid"].text
        print(m_id)
        if(len(m_id) != 0):
            mqtt_c.publish(str(m_id).strip(), "{\"e\":\"AC_CONFIG\"}")

    def ac_setup(self):
        print(self)
        addwindow_instance = self.root.ids.screen_manager.get_screen("device")
        m_id = addwindow_instance.ids["mid"].text
        protocol = addwindow_instance.ids["proto"].text
        model = addwindow_instance.ids["model"].text
        if(len(m_id) != 0):
            if(len(model) != 0):
                if(len(protocol) != 0):
                    print("Publishing data")
                    # REMOTELY CHECKING AC MODEL
                    mqtt_c.publish(str(m_id).strip(),
                                   "{\"AC_MODEL\":\""+model+"\"}")
                    mqtt_c.publish(str(m_id).strip(),
                                   "{\"AC_PROTOCOL\":\""+protocol+"\"}")

    def ac_setup_save(self):

        addwindow_instance = self.root.ids.screen_manager.get_screen("device")
        print(addwindow_instance)
        m_id = addwindow_instance.ids["mid"].text
        print(m_id)
        protocol = addwindow_instance.ids["proto"].text
        model = addwindow_instance.ids["model"].text
        if(len(m_id) != 0):
            if(len(model) != 0):
                if(len(protocol) != 0):
                    # REMOTELY CHECKING AC MODEL
                    mqtt_c.publish(str(m_id).strip(),
                                   "{\"AC_MODEL\":\""+model+"\"}")
                    mqtt_c.publish(str(m_id).strip(),
                                   "{\"AC_PROTOCOL\":\""+protocol+"\"}")
                    mqtt_c.publish(str(m_id).strip(),
                                   "{\"AC_SETUP\":\"YES\"}")

    def ac_restart(self):
        addwindow_instance = self.root.ids.screen_manager.get_screen("device")
        m_id = addwindow_instance.ids["mid"].text
        if(len(m_id) != 0):
            mqtt_c.publish(str(m_id).strip(), "{\"ESP_RESTART\":\"YES\"}")

    def pir_control(self, checkbox, value):
        addwindow_instance = self.root.ids.screen_manager.get_screen("device")
        m_id = addwindow_instance.ids["mid"].text
        print(value)
        if value:
            # PIR ON
            if(len(m_id) != 0):
                mqtt_c.publish(str(m_id).strip(), "{\"PIR\":\"1\"}")
        else:
            # PIR OFF
            if(len(m_id) != 0):
                mqtt_c.publish(str(m_id).strip(), "{\"PIR\":\"0\"}")

    def on_log(self, client, userdata, level, buf):
        print("log: ", buf)  # connect

    def on_message(self, client, userdata, message):

        addwindow_instance = self.root.ids.screen_manager.get_screen("device")

        global dpath

        try:
            print('recevice or send message')
            topic = message.topic
            payload = message.payload.decode()
            print('TOPIC:', topic)
            print('payload:', payload)
            data = json5.loads(payload)

            if (topic == "dubblin/location"):

                latitude = float(data['LAT'])
                longitude = float(data['LON'])
                speed = str(data['SPEED'])
                altitude = str(data['ALT'])
                if(latitude != "NO DATA"):
                    marker = MapMarkerPopup(
                        source="marker.png", lat=latitude, lon=longitude)
                    #addwindow_instance.ids.MAP.lat = latitude
                    #addwindow_instance.ids.MAP.lon = longitude
                    addwindow_instance.ids.MAP.center_on(latitude, longitude)
                    #addwindow_instance.ids.MAP.zoom = 20
                    addwindow_instance.ids.MAP.add_widget(marker)

                addwindow_instance.ids.lat_data.text = f'CURRENT LATITUDE OF VEICHLE :{latitude}'
                addwindow_instance.ids.lon_data.text = f'CURRENT LONGITUDE OF VEICHLE :{longitude}'
                addwindow_instance.ids.speed_data.text = f'CURRENT SPEED OF VEICHLE :{speed}'
                addwindow_instance.ids.alt_data.text = f'CURRENT ALTITUDE OF VEICHLE :{altitude}'
                # addwindow_instance.ids.v_data.pos_hint = {
                # 'center_x': 0.2, 'center_y': 0.1}

        except Exception as e:
            print(e)

   # def Callback_Clock(self, dt):
        # self_heal_count = 0
    def show_data(self, dt):
        if mqtt_c:
            print("Wifi is their")
        else:
            print("Wifi not their")


DemoApp().run()
