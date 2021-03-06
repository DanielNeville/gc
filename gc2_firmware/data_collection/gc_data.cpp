#include "gc_data.h"
#include "common.h"
#include "utils.h"

GcData::GcData(GcClient &gc_client) :
    m_gc_client(gc_client),
    #if USE_IMU_1_BNO055
    m_bno_1(-1, BNO055_ADDRESS_A),
    #endif
    #if USE_IMU_2_MMA8452
    m_mma_2(),
    #endif
    p_battery_charge(0),
    m_last_report_battery_time(-REPORT_BATTERY_INTERVAL),
    m_report_status_battery(false),
    m_simulation_mode(false),
    m_emg_beep(false),
    m_tap_to_upload(false),
    m_fast_movement_start_millis(0),
    m_last_datapoint_time(0),
    m_num_data_points(0){
      memset(&m_last_data_point, 0, sizeof(data_point));
      memset(m_data_points, 0, sizeof(data_point) * STD_DEV_SAMPLES);
}

void GcData::init() {
  //  set pin modes

  if(USE_EMG) {
    pinMode(EMG_SENSOR_PIN, INPUT);
  }
  pinMode(BUZZER_PIN, OUTPUT);

  // initialize various sensors
  pinMode(FAST_MODE_LED_PIN, OUTPUT);

  if(USE_IMU_1_BNO055) {

    if(!m_bno_1.begin())
    {
      DEBUG_LOG("could not find BNO055 1 on I2C bus");
      error_tone();
    } else {
      DEBUG_LOG("BNO055 1 setup OK");
      m_bno_1.setExtCrystalUse(false);
    }
  }

  if (USE_IMU_2_MMA8452) {
    m_mma_2.init(SCALE_2G, ODR_400);
  }

  // setup battery gauge
  lipo.begin();
  lipo.quickStart();

  read_battery_charge();
}

void GcData::read_battery_charge() {
    float battery_charge = lipo.getSOC();
    battery_charge = min(battery_charge, 100.0);
    p_battery_charge = battery_charge;
    m_gc_client.battery_charge(battery_charge);
}

void GcData::get_gyro_1(data_point *dp) {
  int16_t gyro_values[3];
  m_bno_1.getRawVector(Adafruit_BNO055::VECTOR_GYROSCOPE, gyro_values);
  dp->gyro1_x = gyro_values[0];
  dp->gyro1_y = gyro_values[1];
  dp->gyro1_z = gyro_values[2];

  /*
  DEBUG_LOG("gyro: x:" + String(dp->gyro1_x) +
            " y:" + String(dp->gyro1_y) +
            " z:" + String(dp->gyro1_z));
  */
}

void GcData::get_accel_1(int16_t *accel_values) {
  if(! USE_IMU_1_BNO055) {
    accel_values[0] = 0.0;
    accel_values[1] = 0.0;
    accel_values[2] = 0.0;
  } else {
    /*
    imu::Vector<3> accel = m_bno_1.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
    accel_values[0] = accel.x();
    accel_values[1] = accel.y();
    accel_values[2] = accel.z();
    */

    // DEBUG_LOG("accel: x " + String(accel.x()) + " y: " + String(accel.y()) + " z: " + String(accel.z()));

    m_bno_1.getRawVector(Adafruit_BNO055::VECTOR_LINEARACCEL, accel_values);
    //DEBUG_LOG("accel: x " + String(raw_vector[0]) + " y: " + String(raw_vector[1]) + " z: " + String(raw_vector[2]));
  }
}

void GcData::get_accel_2(float *accel_values) {
  if(! USE_IMU_2_MMA8452) {
    accel_values[0] = 0.0;
    accel_values[1] = 0.0;
    accel_values[2] = 0.0;
  } else {
    m_mma_2.read();
    accel_values[0] = m_mma_2.cx * 10.0;
    accel_values[1] = m_mma_2.cy * 10.0;
    accel_values[2] = m_mma_2.cz * 10.0;
  }
}

void GcData::report_battery_charge() {
  DEBUG_LOG("Reporting battery charge");
  read_battery_charge();
  m_last_report_battery_time = millis();
  m_gc_client.report_battery_charge();
}

bool GcData::need_report_battery_charge() {
  if(m_report_status_battery) {
    // status / battery report request
    m_report_status_battery = false;
    return true;
  }
  if (millis() - m_last_report_battery_time > REPORT_BATTERY_INTERVAL) {
    return true;
  }
  return false;
}

void GcData::queue_status_battery_charge() {
  m_report_status_battery = true;
}

bool GcData::tap_received() {
  // check whether we've received a tap, indicating upload was requested
  bool result = false;

  if(USE_IMU_2_MMA8452)
  {
    byte tapStat = m_mma_2.readTap();
    if(tapStat > 0)
    {
      if(m_mma_2.isSingleTap(tapStat)) {
        DEBUG_LOG("received single tap");
      }
      if(m_mma_2.isDoubleTap(tapStat)) {
        DEBUG_LOG("received double tap");
        result = true;
      }
    }
  }

  return result;
}

bool GcData::fast_movement(const data_point &dp1, const data_point &dp2)
{
  if(abs(dp1.imu1_accel_x - dp2.imu1_accel_x) > FAST_MOVEMENT_BNO055_LINEAR_ACCEL )
    return true;
  if(abs(dp1.imu1_accel_y - dp2.imu1_accel_y) > FAST_MOVEMENT_BNO055_LINEAR_ACCEL )
      return true;
  if(abs(dp1.imu1_accel_z - dp2.imu1_accel_z) > FAST_MOVEMENT_BNO055_LINEAR_ACCEL )
      return true;

  if(abs(dp1.imu2_accel_x - dp2.imu2_accel_x) > FAST_MOVEMENT_MMA8452_ACCEL )
    return true;
  if(abs(dp1.imu2_accel_y - dp2.imu2_accel_y) > FAST_MOVEMENT_MMA8452_ACCEL )
      return true;
  if(abs(dp1.imu2_accel_z - dp2.imu2_accel_z) > FAST_MOVEMENT_MMA8452_ACCEL )
      return true;

  return false;
}

void GcData::process_stddev(const data_point &dp)
{
  if(m_num_data_points < STD_DEV_SAMPLES)
  {
    m_data_points[m_num_data_points] = dp; // copy
    m_num_data_points++;
  } else {
    // time to compute stddev

    // 1. sum
    std_dev sums;
    memset(&sums, 0, sizeof(std_dev));
    for(int i = 0; i < STD_DEV_SAMPLES;  i++)
    {
      sums.gyro1_x += m_data_points[i].gyro1_x;
      sums.gyro1_y += m_data_points[i].gyro1_y;
      sums.gyro1_z += m_data_points[i].gyro1_z;

      sums.imu2_accel_x += m_data_points[i].imu2_accel_x;
      sums.imu2_accel_y += m_data_points[i].imu2_accel_y;
      sums.imu2_accel_z += m_data_points[i].imu2_accel_z;
    }
    // 2. means
    std_dev means;
    memset(&means, 0, sizeof(std_dev));
    means.gyro1_x = sums.gyro1_x / STD_DEV_SAMPLES;
    means.gyro1_y = sums.gyro1_y / STD_DEV_SAMPLES;
    means.gyro1_z = sums.gyro1_z / STD_DEV_SAMPLES;
    means.imu2_accel_x = sums.imu2_accel_x / STD_DEV_SAMPLES;
    means.imu2_accel_y = sums.imu2_accel_y / STD_DEV_SAMPLES;
    means.imu2_accel_z = sums.imu2_accel_z / STD_DEV_SAMPLES;

    // 3. deviations
    std_dev deviations;
    memset(&deviations, 0, sizeof(std_dev));
    for(int i = 0; i < STD_DEV_SAMPLES;  i++)
    {
      deviations.gyro1_x = pow(m_data_points[i].gyro1_x - means.gyro1_x, 2);
      deviations.gyro1_y = pow(m_data_points[i].gyro1_y - means.gyro1_y, 2);
      deviations.gyro1_z = pow(m_data_points[i].gyro1_z - means.gyro1_z, 2);

      deviations.imu2_accel_x = pow(m_data_points[i].imu2_accel_x - means.imu2_accel_x, 2);
      deviations.imu2_accel_y = pow(m_data_points[i].imu2_accel_y - means.imu2_accel_y, 2);
      deviations.imu2_accel_z = pow(m_data_points[i].imu2_accel_z - means.imu2_accel_z, 2);

    }

    // 4. std deviation
    std_dev std_devs;
    memset(&std_devs, 0, sizeof(std_dev));
    std_devs.gyro1_x = sqrt(deviations.gyro1_x / STD_DEV_SAMPLES);
    std_devs.gyro1_y = sqrt(deviations.gyro1_y / STD_DEV_SAMPLES);
    std_devs.gyro1_z = sqrt(deviations.gyro1_z / STD_DEV_SAMPLES);
    std_devs.imu2_accel_x = sqrt(deviations.imu2_accel_x / STD_DEV_SAMPLES);
    std_devs.imu2_accel_y = sqrt(deviations.imu2_accel_y / STD_DEV_SAMPLES);
    std_devs.imu2_accel_z = sqrt(deviations.imu2_accel_z / STD_DEV_SAMPLES);

    DEBUG_LOG(String("stddev: ") +
              " gyro1_x: " + String(std_devs.gyro1_x) +
              " gyro1_y: " + String(std_devs.gyro1_y) +
              " gyro1_z: " + String(std_devs.gyro1_z) +
              " imu2_accel_x: " + String(std_devs.imu2_accel_x) +
              " imu2_accel_y: " + String(std_devs.imu2_accel_y) +
              " imu2_accel_z: " + String(std_devs.imu2_accel_z)
            );

    m_num_data_points = 0;

    std_devs.milliseconds = millis();
    m_gc_client.add_stddev(std_devs);
  }
}

void GcData::collect_data(bool upload_requested) {

  data_point dp;
  memset(&dp, 0, sizeof(data_point));

  // eventually this will overflow, but the device is only on for one night,
  // so should be OK
  dp.milliseconds = millis();

  get_gyro_1(&dp);

  int16_t accel1_values[3];
  float accel2_values[3];

  get_accel_1(accel1_values);
  get_accel_2(accel2_values);

  dp.imu1_accel_x = accel1_values[0];
  dp.imu1_accel_y = accel1_values[1];
  dp.imu1_accel_z = accel1_values[2];

  dp.imu2_accel_x = accel2_values[0] * 1000.0;
  dp.imu2_accel_y = accel2_values[1] * 1000.0;
  dp.imu2_accel_z = accel2_values[2] * 1000.0;

  dp.flags1 = 0;
  dp.flags2 = 0;

  uint32_t current_millis = millis();
  // check whether fast movement needs to be turned on or off
  if(fast_movement(dp, m_last_data_point))
  {
    if(m_fast_movement_start_millis == 0) {
      // turning on fast movement
      digitalWrite(FAST_MODE_LED_PIN, HIGH);
      DEBUG_LOG("turning on fast movement");
    }
    m_fast_movement_start_millis = millis();
  } else {
    // is fast movement on right now ?
    if (m_fast_movement_start_millis > 0) {
      if(current_millis - m_fast_movement_start_millis > FAST_MOVEMENT_DURATION) {
        // turn it off
        digitalWrite(FAST_MODE_LED_PIN, LOW);
        m_fast_movement_start_millis = 0;
        DEBUG_LOG("turning off fast movement");
      }
    }
  }

  if (m_fast_movement_start_millis > 0 ||
      current_millis - m_last_datapoint_time > 2000) {
      m_gc_client.add_datapoint(dp);
      m_last_datapoint_time = current_millis;
  }

  m_last_data_point = dp;

  process_stddev(dp);

  if(need_report_battery_charge()){
    report_battery_charge();
  }

  bool tapReceived = tap_received();
  if (tapReceived && m_tap_to_upload) {
    validation_tone();
    upload_requested = true;
  }

  if(m_gc_client.need_upload() || upload_requested){
    DEBUG_LOG("need to upload batch");
    m_gc_client.upload_batch();
  }
}

void GcData::enable_tap_to_upload(bool enable)
{
  m_tap_to_upload = enable;
}
