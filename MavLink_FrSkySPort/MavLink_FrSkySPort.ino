
/*
APM2.5 Mavlink to FrSky X8R SPort interface using Teensy 3.1  http://www.pjrc.com/teensy/index.html
based on ideas found here http://code.google.com/p/telemetry-convert/
******************************************************
Cut board on the backside to separate Vin from VUSB

Connection on Teensy 3.1:
SPort S --> TX1
SPort + --> Vin
SPort  - --> GND

APM Telemetry DF13-5  Pin 2 --> RX2
APM Telemetry DF13-5  Pin 3 --> TX2
APM Telemetry DF13-5  Pin 5 --> GND

Analog input  --> A0 (pin14) on Teensy 3.1 ( max 3.3 V )


This is the data we send to FrSky, you can change this to have your own
set of data
******************************************************
Data transmitted to FrSky Taranis:
Cell           ( Voltage of Cell=Cells/4. [V] This is my LiPo pack 4S )
Cells         ( Voltage from LiPo [V] )
A2             ( Analog voltage from input A0 on Teensy 3.1 )
Alt             ( Altitude from baro.  [m] )
GAlt          ( Altitude from GPS   [m])
HDG         ( Compass heading  [deg])
Rpm         ( Throttle when ARMED [%] )
AccX         ( AccX m/s ? )
AccY         ( AccY m/s ? )
AccZ         ( AccZ m/s ? )
VSpd        ( Vertical speed [m/s] )
Speed      ( Ground speed from GPS,  [km/h] )
T1            ( GPS status = ap_sat_visible*10) + ap_fixtype )
T2            ( ARMED=1, DISARMED=0 )
Vfas          ( same as Cells )
Longitud
Latitud
Dist          ( Will be calculated by FrSky Taranis as the distance from first received lat/long = Home Position

******************************************************

*/

#include <GCS_MAVLink.h>
#include "FrSkySPort.h"
// LMD START
#include "LightAnim.h"

#define MAVLINK_ENABLED 0
// LMD END

#define debugSerial           Serial
#define _MavLinkSerial      Serial2
#define START                   1
#define MSG_RATE            10              // Hertz

// ******************************************
// Message #0  HEARTHBEAT
uint8_t    ap_type = 0;
uint8_t    ap_autopilot = 0;
uint8_t    ap_base_mode = 0;
uint32_t  ap_custom_mode = 0;
uint8_t    ap_system_status = 0;
uint8_t    ap_mavlink_version = 0;

// Message # 1  SYS_STATUS
uint16_t  ap_voltage_battery = 0;    // 1000 = 1V
int16_t    ap_current_battery = 0;    //  10 = 1A

// Message #24  GPS_RAW_INT
uint8_t    ap_fixtype = 3;                  //   0= No GPS, 1 = No Fix, 2 = 2D Fix, 3 = 3D Fix
uint8_t    ap_sat_visible = 0;           // numbers of visible satelites
// FrSky Taranis uses the first recieved lat/long as homeposition.
int32_t    ap_latitude = 0;              // 585522540;
int32_t    ap_longitude = 0;            // 162344467;
int32_t    ap_gps_altitude = 0;        // 1000 = 1m


// Message #74 VFR_HUD
int32_t    ap_airspeed = 0;
uint32_t  ap_groundspeed = 0;
uint32_t  ap_heading = 0;
uint16_t  ap_throttle = 0;

// FrSky Taranis uses the first recieved value after 'PowerOn' or  'Telemetry Reset'  as zero altitude
int32_t    ap_bar_altitude = 0;    // 100 = 1m
int32_t    ap_climb_rate = 0;      // 100= 1m/s

// Message #27 RAW IMU
int32_t   ap_accX = 0;
int32_t   ap_accY = 0;
int32_t   ap_accZ = 0;

int32_t   ap_accX_old = 0;
int32_t   ap_accY_old = 0;
int32_t   ap_accZ_old = 0;

// ******************************************
// These are special for FrSky
int32_t   adc2 = 0;               // 100 = 1.0V
int32_t     vfas = 0;                // 100 = 1,0V
int32_t     gps_status = 0;     // (ap_sat_visible * 10) + ap_fixtype
// ex. 83 = 8 sattelites visible, 3D lock
uint8_t   ap_cell_count = 0;

// ******************************************
uint8_t     MavLink_Connected;
uint8_t     buf[MAVLINK_MAX_PACKET_LEN];

uint16_t  hb_count;

unsigned long MavLink_Connected_timer;
unsigned long hb_timer;
unsigned long acc_timer;

int led = 13;

// LMD START

LightAnim lightshow;
// LMD END

mavlink_message_t msg;

// ******************************************
void setup()  {

  // LMD START
  if ( MAVLINK_ENABLED ) {
    // LMD END

    FrSkySPort_Init();
    _MavLinkSerial.begin(57600);
    debugSerial.begin(57600);
    MavLink_Connected = 0;
    MavLink_Connected_timer = millis();
    hb_timer = millis();
    acc_timer = millis();
    hb_count = 0;


    pinMode(led, OUTPUT);
    pinMode(12, OUTPUT);

    pinMode(14, INPUT);
    analogReference(DEFAULT);

    // LMD START
  }

  lightshow.begin();
  // LMD END

}


// ******************************************
void loop()  {
  uint16_t len;

  // LMD START
  if ( MAVLINK_ENABLED ) {
    // LMD END

    if (millis() - hb_timer > 1500) {
      hb_timer = millis();
      if (!MavLink_Connected) {   // Start requesting data streams from MavLink
        digitalWrite(led, HIGH);
        mavlink_msg_request_data_stream_pack(0xFF, 0xBE, &msg, 1, 1, MAV_DATA_STREAM_EXTENDED_STATUS, MSG_RATE, START);
        len = mavlink_msg_to_send_buffer(buf, &msg);
        _MavLinkSerial.write(buf, len);
        delay(10);
        mavlink_msg_request_data_stream_pack(0xFF, 0xBE, &msg, 1, 1, MAV_DATA_STREAM_EXTRA2, MSG_RATE, START);
        len = mavlink_msg_to_send_buffer(buf, &msg);
        _MavLinkSerial.write(buf, len);
        delay(10);
        mavlink_msg_request_data_stream_pack(0xFF, 0xBE, &msg, 1, 1, MAV_DATA_STREAM_RAW_SENSORS, MSG_RATE, START);
        len = mavlink_msg_to_send_buffer(buf, &msg);
        _MavLinkSerial.write(buf, len);
        digitalWrite(led, LOW);
      }
    }

    if (MavLink_Connected && (millis() - MavLink_Connected_timer) > 1500)  {  // if no HEARTBEAT from APM  in 1.5s then we are not connected
      MavLink_Connected = 0;
      hb_count = 0;
    }

    _MavLink_receive();                   // Check MavLink communication

    FrSkySPort_Process();               // Check FrSky S.Port communication

    adc2 = analogRead(0) / 4;            // Read ananog value from A0 (Pin 14). ( Will be A2 value on FrSky LCD)

    if ((millis() - acc_timer) > 1000) {   // Reset timer for AccX, AccY and AccZ
      ap_accX_old = ap_accX;
      ap_accY_old = ap_accY;
      ap_accZ_old = ap_accZ;
      acc_timer = millis();
      //      debugSerial.print("Base mode: ");
      //      debugSerial.println(ap_base_mode);

    }

    // LMD START
  }

  lightshow.loop();
  // LMD END
}


void _MavLink_receive() {
  mavlink_message_t msg;
  mavlink_status_t status;

  while (_MavLinkSerial.available())
  {
    uint8_t c = _MavLinkSerial.read();
    if (mavlink_parse_char(MAVLINK_COMM_0, c, &msg, &status))
    {
      switch (msg.msgid)
      {
        case MAVLINK_MSG_ID_HEARTBEAT:  // 0
          ap_base_mode = (mavlink_msg_heartbeat_get_base_mode(&msg) & 0x80) > 7;
          ap_custom_mode = mavlink_msg_heartbeat_get_custom_mode(&msg);
          MavLink_Connected_timer = millis();
          if (!MavLink_Connected); {
            hb_count++;
            if ((hb_count++) > 10) {       // If  received > 10 heartbeats from MavLink then we are connected
              MavLink_Connected = 1;
              hb_count = 0;
              digitalWrite(led, HIGH);     // LED will be ON when connected to MavLink, else it will slowly blink
            }
          }
          break;
        case MAVLINK_MSG_ID_SYS_STATUS :   // 1
          ap_voltage_battery = Get_Volt_Average(mavlink_msg_sys_status_get_voltage_battery(&msg));  // 1 = 1mV
          ap_current_battery = Get_Current_Average(mavlink_msg_sys_status_get_current_battery(&msg));     // 1=10mA

          if (ap_voltage_battery > 21000) ap_cell_count = 6;
          else if (ap_voltage_battery > 16800 && ap_cell_count != 6) ap_cell_count = 5;
          else if (ap_voltage_battery > 12600 && ap_cell_count != 5) ap_cell_count = 4;
          else if (ap_voltage_battery > 8400 && ap_cell_count != 4) ap_cell_count = 3;
          else if (ap_voltage_battery > 4200 && ap_cell_count != 3) ap_cell_count = 2;
          else ap_cell_count = 0;
          break;
        case MAVLINK_MSG_ID_GPS_RAW_INT:   // 24
          ap_fixtype = mavlink_msg_gps_raw_int_get_fix_type(&msg);                               // 0 = No GPS, 1 =No Fix, 2 = 2D Fix, 3 = 3D Fix
          ap_sat_visible =  mavlink_msg_gps_raw_int_get_satellites_visible(&msg);          // numbers of visible satelites
          gps_status = (ap_sat_visible * 10) + ap_fixtype;
          if (ap_fixtype == 3)  {
            ap_latitude = mavlink_msg_gps_raw_int_get_lat(&msg);
            ap_longitude = mavlink_msg_gps_raw_int_get_lon(&msg);
            ap_gps_altitude = mavlink_msg_gps_raw_int_get_alt(&msg);    // 1m =1000
          }
          break;
        case MAVLINK_MSG_ID_RAW_IMU:   // 27
          ap_accX = mavlink_msg_raw_imu_get_xacc(&msg) / 10;                //
          ap_accY = mavlink_msg_raw_imu_get_yacc(&msg) / 10;
          ap_accZ = mavlink_msg_raw_imu_get_zacc(&msg) / 10;
          break;
        case MAVLINK_MSG_ID_VFR_HUD:   //  74
          ap_airspeed = 0;
          ap_groundspeed = mavlink_msg_vfr_hud_get_groundspeed(&msg);      // 100 = 1m/s
          ap_heading = mavlink_msg_vfr_hud_get_heading(&msg);     // 100 = 100 deg
          ap_throttle = mavlink_msg_vfr_hud_get_throttle(&msg);        //  100 = 100%
          ap_bar_altitude = mavlink_msg_vfr_hud_get_alt(&msg) * 100;        //  m
          ap_climb_rate = mavlink_msg_vfr_hud_get_climb(&msg) * 100;      //  m/s
          break;
        default:
          break;
      }
    }
  }
}


