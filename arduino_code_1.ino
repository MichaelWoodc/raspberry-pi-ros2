#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <micro_ros_arduino.h>

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/int32.h>
#include <std_msgs/msg/string.h>
#include <geometry_msgs/msg/twist.h>
#include <sensor_msgs/msg/imu.h>

#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

// ---------------- WiFi ----------------
#define WIFI_SSID "OpenWrt2.4"
#define WIFI_PASS "narrowboat564"

#define AGENT_IP   "192.168.1.135"
#define AGENT_PORT 8888

// ---------------- Motor Pins (TB6600) ----------------
#define LEFT_STEP   4
#define LEFT_DIR    5

#define RIGHT_STEP  18
#define RIGHT_DIR   19

#define ENABLE_PIN  21

// ---------------- Motor Settings ----------------
#define STEPS_PER_REV   200.0
#define MICROSTEP       16.0
#define WHEEL_RADIUS_M  0.05
#define WHEEL_BASE_M    0.30

#define STEPS_PER_METER ((STEPS_PER_REV * MICROSTEP) / (2 * PI * WHEEL_RADIUS_M))

// ---------------- IMU (BNO055) ----------------
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

// ---------------- ROS ----------------
rcl_allocator_t allocator;
rclc_support_t support;
rcl_node_t node;

rcl_publisher_t heartbeat_pub;
rcl_publisher_t debug_pub;
rcl_publisher_t imu_pub;

std_msgs__msg__Int32 heartbeat_msg;
std_msgs__msg__String debug_msg;
sensor_msgs__msg__Imu imu_msg;

rcl_subscription_t cmd_vel_sub;
geometry_msgs__msg__Twist cmd_vel_msg;

rclc_executor_t executor;

float linear_x = 0;
float angular_z = 0;
unsigned long last_cmd_time = 0;

// ---------------- cmd_vel callback ----------------
void cmd_vel_callback(const void * msgin)
{
  const geometry_msgs__msg__Twist * msg =
      (const geometry_msgs__msg__Twist *)msgin;

  linear_x = msg->linear.x;
  angular_z = msg->angular.z;

  last_cmd_time = millis();

  Serial.printf("cmd_vel -> linear: %.2f angular: %.2f\n",
                linear_x, angular_z);
}

// ---------------- Motor Control ----------------
void setMotor(int stepPin, int dirPin, float speed_m_s)
{
  if (speed_m_s == 0)
    return;

  digitalWrite(dirPin, speed_m_s > 0 ? HIGH : LOW);

  float steps_per_sec = fabs(speed_m_s) * STEPS_PER_METER;
  float delay_us = 1e6 / steps_per_sec / 2;

  digitalWrite(stepPin, HIGH);
  delayMicroseconds(delay_us);
  digitalWrite(stepPin, LOW);
  delayMicroseconds(delay_us);
}

void driveMotors()
{
  if (millis() - last_cmd_time > 500)
  {
    linear_x = 0;
    angular_z = 0;
  }

  float v_left  = linear_x - angular_z * (WHEEL_BASE_M / 2.0);
  float v_right = linear_x + angular_z * (WHEEL_BASE_M / 2.0);

  setMotor(LEFT_STEP, LEFT_DIR, v_left);
  setMotor(RIGHT_STEP, RIGHT_DIR, v_right);
}

// ---------------- IMU (BNO055) ----------------
void publishIMU()
{
  imu::Quaternion quat = bno.getQuat();

  imu_msg.orientation.w = quat.w();
  imu_msg.orientation.x = quat.x();
  imu_msg.orientation.y = quat.y();
  imu_msg.orientation.z = quat.z();

  sensors_event_t gyro_event;
  bno.getEvent(&gyro_event, Adafruit_BNO055::VECTOR_GYROSCOPE);

  imu_msg.angular_velocity.x = gyro_event.gyro.x;
  imu_msg.angular_velocity.y = gyro_event.gyro.y;
  imu_msg.angular_velocity.z = gyro_event.gyro.z;

  sensors_event_t accel_event;
  bno.getEvent(&accel_event, Adafruit_BNO055::VECTOR_LINEARACCEL);

  imu_msg.linear_acceleration.x = accel_event.acceleration.x;
  imu_msg.linear_acceleration.y = accel_event.acceleration.y;
  imu_msg.linear_acceleration.z = accel_event.acceleration.z;

  rcl_publish(&imu_pub, &imu_msg, NULL);
}

// ---------------- setup ----------------
void setup()
{
  Serial.begin(115200);
  delay(2000);
  Serial.println("BOOTING...");

  pinMode(LEFT_STEP, OUTPUT);
  pinMode(LEFT_DIR, OUTPUT);
  pinMode(RIGHT_STEP, OUTPUT);
  pinMode(RIGHT_DIR, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, LOW);

  // IMU
  if (!bno.begin())
  {
    Serial.println("BNO055 NOT detected!");
    while (1) delay(100);
  }
  bno.setExtCrystalUse(true);
  Serial.println("IMU OK");

  // WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi OK");
  Serial.println(WiFi.localIP());

  // micro-ROS
  set_microros_wifi_transports(WIFI_SSID, WIFI_PASS, AGENT_IP, AGENT_PORT);

  allocator = rcl_get_default_allocator();
  rclc_support_init(&support, 0, NULL, &allocator);
  rclc_node_init_default(&node, "esp32_robot", "", &support);

  // QoS: BEST_EFFORT for Wi-Fi micro-ROS publishers
  rclc_publisher_init_best_effort(
    &heartbeat_pub,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
    "esp32/heartbeat");

  rclc_publisher_init_best_effort(
    &debug_pub,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),
    "esp32/debug");

  rclc_publisher_init_best_effort(
    &imu_pub,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Imu),
    "imu/data");

  // Subscriber (default QoS is fine)
  rclc_subscription_init_default(
    &cmd_vel_sub,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist),
    "cmd_vel");

  // Executor
  rclc_executor_init(&executor, &support.context, 4, &allocator);
  rclc_executor_add_subscription(
    &executor,
    &cmd_vel_sub,
    &cmd_vel_msg,
    &cmd_vel_callback,
    ON_NEW_DATA);

  heartbeat_msg.data = 0;

  debug_msg.data.data = (char*)malloc(100);
  debug_msg.data.capacity = 100;
  debug_msg.data.size = 0;

  Serial.println("micro-ROS ready");
}

// ---------------- loop ----------------
void loop()
{
  rclc_executor_spin_some(&executor, RCL_MS_TO_NS(5));

  driveMotors();
  publishIMU();

  static unsigned long last_hb = 0;
  if (millis() - last_hb > 1000)
  {
    last_hb = millis();
    heartbeat_msg.data++;

    snprintf(
      debug_msg.data.data,
      100,
      "HB=%d L=%.2f A=%.2f",
      heartbeat_msg.data,
      linear_x,
      angular_z
    );

    debug_msg.data.size = strlen(debug_msg.data.data);

    rcl_publish(&heartbeat_pub, &heartbeat_msg, NULL);
    rcl_publish(&debug_pub, &debug_msg, NULL);

    Serial.printf("HEARTBEAT #%d\n", heartbeat_msg.data);
  }
}
