
#include <math.h>

/* ---------- เครื่องสุ่ม (พอร์ตข้ามบอร์ด) ---------- */
#if defined(ESP32)
#include "esp_random.h"
static uint32_t rng32() {
  return esp_random();
}  // ฮาร์ดแวร์ RNG
#else
static uint32_t rng32() {  // AVR fallback
  return ((uint32_t)random(0, 65536) << 16) | (uint32_t)random(0, 65536);
}
#endif

/* สุ่ม uniform ในช่วง [lo, hi] */
static float frand(float lo, float hi) {
  return lo + (hi - lo) * ((float)rng32() / (float)UINT32_MAX);
}

static float clampf(float v, float lo, float hi) {
  return v < lo ? lo : (v > hi ? hi : v);
}

/* แบบ Arduino map() แต่เป็น float (เผื่อใช้ map เชิงเส้นตรง ๆ) */
static float map_f(float x, float in_min, float in_max,
                   float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/* ---------- ตาราง breakpoint AQI ของ PM2.5 (EPA 2024) ---------- */
typedef struct {
  float clo, chi;  // ช่วงความเข้มข้น µg/m3
  int ilo, ihi;    // ช่วงค่า AQI
} AqiBand;

static const AqiBand PM25_BANDS[] = {
  { 0.0f, 9.0f, 0, 50 },         // Good
  { 9.1f, 35.4f, 51, 100 },      // Moderate
  { 35.5f, 55.4f, 101, 150 },    // USG
  { 55.5f, 125.4f, 151, 200 },   // Unhealthy
  { 125.5f, 225.4f, 201, 300 },  // Very Unhealthy
  { 225.5f, 325.4f, 301, 500 },  // Hazardous
};
static const int N_BANDS = sizeof(PM25_BANDS) / sizeof(PM25_BANDS[0]);

/* ---------- PM2.5 -> AQI (สูตรจริง EPA) ---------- */
int pm25_to_aqi(float pm) {
  if (pm < 0) pm = 0;
  float c = floorf(pm * 10.0f) / 10.0f;  // truncate 1 ทศนิยม ตาม EPA
  for (int i = 0; i < N_BANDS; i++) {
    if (c <= PM25_BANDS[i].chi) {
      AqiBand b = PM25_BANDS[i];
      float aqi = (float)(b.ihi - b.ilo) / (b.chi - b.clo) * (c - b.clo) + b.ilo;
      return (int)lroundf(aqi);
    }
  }
  return 500;  // เกินสเกล -> cap 500
}

int aqi_level(int aqi) {
  if (aqi <= 50) return 1;
  if (aqi <= 100) return 2;
  if (aqi <= 150) return 3;
  if (aqi <= 200) return 4;
  if (aqi <= 300) return 5;
  return 0;
}

/* ---------- โครงสร้างผล + ฟังก์ชัน map หลัก ---------- */
typedef struct {
  float pm25;  // µg/m3
  float o3;    // ppb
  float tvoc;  // ppb
  float co2;   // ppm
  int aqi;
} AirSample;

AirSample air_from_pm(float pm25) {
  AirSample s;
  s.pm25 = pm25;
  //              base    + slope*PM        + noise            clamp(min,max)
  s.o3 = clampf(15.0f + 0.25f * pm25 + frand(-20, 20), 0.0f, 200.0f);       // ppb
  s.tvoc = clampf(80.0f + 1.20f * pm25 + frand(-60, 60), 0.0f, 2000.0f);    // ppb
  s.co2 = clampf(420.0f + 2.00f * pm25 + frand(-80, 80), 400.0f, 2500.0f);  // ppm
  s.aqi = pm25_to_aqi(pm25);
  return s;
}

/* ---------- พิมพ์ผลออก Serial ---------- */
void printSample(AirSample s) {
  Serial.print("PM2.5=");
  Serial.print(s.pm25, 1);
  Serial.print("  O3=");
  Serial.print(s.o3, 1);
  Serial.print("  TVOC=");
  Serial.print(s.tvoc, 1);
  Serial.print("  CO2=");
  Serial.print(s.co2, 1);
  Serial.print("  AQI=");
  Serial.print(s.aqi);
  Serial.print("  [");
  Serial.print(aqi_level(s.aqi));
  Serial.println("]");

  o3_ppm = s.o3/1000;
  tvoc = s.tvoc/1000;
  eco2 = s.co2/1000;
  aqi = s.aqi/1000;
}



void datax() {
  float pm = pmx;
  AirSample s = air_from_pm(pm);
  printSample(s);
}
