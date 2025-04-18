#include "time.h"

char* days[8] = {"MON","TUE","WED","THU","FRI","SAT","SUN",""};
char* get_wday_shortcut(int day) {
  return days[day&7];
}

int tm_to_seconds(tm_t *t) {
  return ((int)t->hour*60 + t->min)*60 + t->sec;
}

int tm_to_cseconds(tm_t *t) {
  return t->csec + 100*tm_to_seconds(t);
}

void seconds_to_tm(int seconds, tm_t *t) {
  t->sec = seconds % 60;
  t->min = (seconds / 60) % 60;
  t->hour = seconds / 3600;
}

void cseconds_to_tm(int cseconds, tm_t *t) {
  t->csec = cseconds % 100;
  seconds_to_tm(cseconds/100, t);
}


int dt_equal(dt_t *a, dt_t *b) {
  return a->day == b->day && a->month == b->month && a->year == b->year;
}



// ----------------------------------------------------------
//   Julian day number calculation (Gregorian)
// ----------------------------------------------------------
// a = (14-month)/12     # 1 for Jan/Feb, 0 for others
// y = year + 4800 - a   # nr. of years since March 1 -4800 (Mar 1, 4801 BC)
// m = month + 12a - 3   # 0 for Mar, ..., 11 for Feb
// day + (153m+2)/5 + 365y + y/4 - y/100 + y/400 - 32045
// ----------------------------------------------------------
int julian_day(dt_t *d) {
  int a = (d->month<3) ? 1 : 0; // 1 for Jan/Feb, 0 for others
  int y = d->year + 4800 - a;   // nr. of years since March 1 -4800 (Mar 1, 4801 BC)
  int m = d->month + 12*a - 3;  // 0 for Mar, ..., 11 for Feb
  return  (153*m+2)/5 + 365*y + y/4 - y/100 + y/400 - 32045 + d->day;
}




// ----------------------------------------------------------
//   Julian day to date calculation (Gregorian)
// ----------------------------------------------------------
// Algorithm parameters for Gregorian calendar
//  var val   var val
//   y  4716   v   3
//   j  1401   u   5
//   m  2      s   153
//   n  12     w   2
//   r  4      B   274277
//   p  1461   C  −38
// ----------------------------------------------------------
// 1. f = J + j + (((4 * J + B) div 146097) * 3) div 4 + C
// 2. e = r * f + v
// 3. g = mod(e, p) div r
// 4. h = u * g + w
// 5. D = (mod(h, s)) div u + 1
// 6. M = mod(h div s + m, n) + 1
// 7. Y = (e div p) - y + (n + m - M) div n
// ----------------------------------------------------------

#define  y   4716
#define  v      3
#define  j   1401
#define  u      5
#define  m      2
#define  s    153
#define  n     12
#define  w      2
#define  r      4
#define  B 274277
#define  p   1461
#define  C    -38

#define div /
#define mod(a,b) ((a) % (b))

void julian_to_date(int julian_date, dt_t *d) {
  int f = julian_date + j + (((4 * julian_date + B) div 146097) * 3) div 4 + C;
  int e = r * f + v;
  int g = mod(e, p) div r;
  int h = u * g + w;
  d->day = (mod(h, s)) div u + 1;
  d->month = mod(h div s + m, n) + 1;
  d->year = (e div p) - y + (n + m - d->month) div n;
}

