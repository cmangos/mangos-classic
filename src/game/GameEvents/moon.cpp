///////////////////////////////////////////////////////////////
///
///  moon.c++
/// 
/// 
///  Created March 31, 2004 by Don Dugger
/// 
///
///////////////////////////////////////////////////////////////
///
/// THE SOFTWARE IS PROVIDED ~AS IS~, WITHOUT WARRANTY OF ANY KIND,
/// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
/// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
/// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
/// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
/// DEALINGS IN THE SOFTWARE.
///
///////////////////////////////////////////////////////////////

#include "moon.h"

///////////////////////////////////////////////////////////////
const double Moon::RAD = (3.1415926535897/180.0);
const long   Moon::IGREG = (15+31L*(10+12L*1582));
const double Moon::MEAN_LUNAR_CYCLE = 29.53058868;
///////////////////////////////////////////////////////////////

double Moon::phase(int mon,int day,int year)
{
    // the new moon Julian date
    long    nm_jd = 0;
    double  nm_frac = 0.0;
    // the 1st quarter Julian date
    long    fq_jd = 0;
    double  fq_frac = 0.0;
    // the full moon Julian date
    long    fm_jd = 0;
    double  fm_frac = 0.0;
    // the last quarter Julian date
    long    lq_jd = 0;
    double  lq_frac = 0.0;
    // The Julian date entered
    long    jd = 0;
    double  frac = 0.0;
    // The lunar phase
    double  ph = 0.0;
    // The portion of a lunar phase which elapse per day
    double phase_per_day = 1/(MEAN_LUNAR_CYCLE/4);
    // 12.37 is the number of lunar cycles per year
    // n then is the number of lunar cycles since 1900 and the year month entered
    int n = (int)(12.37*((year-1900)+((mon-0.5)/12.0)));

    // Get the Julian Date and make sure date entered is valid
    if(!(jd = Calendar2Julian(mon,day,year))) return(-1.0);
    // Now get the first date of the new moon of year mouth entered
    flmoon(n,0,nm_jd,nm_frac);
    // If it after the entered date backup one lunar cycle
    if(jd < nm_jd) flmoon(--n,0,nm_jd,nm_frac);
    // Now find the 1st querter
    flmoon(n,1,fq_jd,fq_frac);
    // If it's after or on the day entered 
    if(fq_jd >= jd) {
        // Now calculate the phase which we now know is between the
        // new moon and first quarter 
        if(abs((int)jd - nm_jd) < abs((int)jd - fq_jd)) {
            // We are closer to the new moon so no phase offset needed 
            ph = (double)(((double)(jd) - ((double)nm_jd+nm_frac)) * phase_per_day);
            if(ph < 0) ph = 4.0 + ph;
            return(ph > 4.0 ? ph - 4.0 : ph); // Keep the value between 0-4
        } else {
            // we are closer to the 1st quarter so add it in
            ph = 1.0+(double)(((double)(jd) - ((double)fq_jd+fq_frac)) * phase_per_day);
            return(ph > 4.0 ? ph - 4.0 : ph); // Keep the value between 0-4
        }
    }
    // Now find the full moon querter
    flmoon(n,2,fm_jd,fm_frac);
    // If it's after or on the day entered 
    if(fm_jd >= jd) {
        // Now calculate the phase which we now know is between the
        // first quarter and the full moon 
        if(abs((int)jd - fq_jd) < abs((int)jd - fm_jd)) {
            // we are closer to the 1st quarter so add it in
            ph = 1.0+(double)(((double)(jd) - ((double)fq_jd+fq_frac)) * phase_per_day);
            return(ph > 4.0 ? ph - 4.0 : ph); // Keep the value between 0-4
        } else {
            // we are closer to the full moon so add it in
            ph = 2.0+(double)(((double)(jd) - ((double)fm_jd+fm_frac)) * phase_per_day);
            return(ph > 4.0 ? ph - 4.0 : ph); // Keep the value between 0-4
        }
    }
    // Now find the last querter
    flmoon(n,3,lq_jd,lq_frac);
    // If it's after or on the day entered 
    if(lq_jd >= jd) {
        // Now calculate the phase which we now know is between the
        // full moon and last quarter
        if(abs((int)jd - fm_jd) < abs((int)jd - lq_jd)) {
            // we are closer to the full moon so add it in
            ph = (2.0+(double)(((double)(jd) - ((double)fm_jd+fm_frac)) * phase_per_day));
            return(ph > 4.0 ? ph - 4.0 : ph); // Keep the value between 0-4
        } else {
            // we are closer to the last quarter so add it in
            ph = (3.0+(double)(((double)(jd) - ((double)lq_jd+lq_frac)) * phase_per_day));
            return(ph > 4.0 ? ph - 4.0 : ph); // Keep the value between 0-4
        }
    }
    // Now find the new moon querter
    flmoon(++n,0,nm_jd,nm_frac);
    // Now calculate the phase which we now know is after the last quarter
    ph = (4.0+(double)(((double)(jd) - ((double)nm_jd+nm_frac)) * phase_per_day));
    return(ph > 4.0 ? ph - 4.0 : ph); // Keep the value between 0-4

} // End of phase()

///////////////////////////////////////////////////////////////

void Moon::nextphase(int& mon,int& day,int& year,int& hr,int& min,int nph)
{
    long jd = 0;
    double djd = 0.0;
    long ph_jd = 0;
    double frac = 0.0;
    int n = (int)(12.37*((year-1900)+((mon-0.5)/12.0)));
    double intpart = 0.0;

    jd = Calendar2Julian(mon,day,year);
    djd = flmoon(n,nph,ph_jd,frac) + 0.5;
    while(jd < ph_jd) {
        djd = flmoon(--n,nph,ph_jd,frac) + 0.5;
    }
    while(jd > ph_jd) {
        djd = flmoon(++n,nph,ph_jd,frac) + 0.5;
    }

    struct tm *tmpt;
    time_t e;

    e = time(NULL);
    tmpt = localtime(&e);
    // djd += ((double)tmpt->tm_gmtoff * (1.0/86400.0));
    if(tmpt->tm_isdst) djd -= (1.0/24.0);
    ph_jd = (long)floor(djd);
    if(isDST(jd)) {
        djd += (1.0/24.0);
        ph_jd = (long)floor(djd);
    }
    frac = djd - floor(djd);
    
    Julian2Calendar(ph_jd,mon,day,year);
    frac *= 24.0;
    hr = (int)(frac >= 0.0 ? floor(frac) : ceil(frac-1.0));
    frac -= (double)hr;
    min = (int)floor(60*frac);

} // End of nextphase()

///////////////////////////////////////////////////////////////

double Moon::flmoon(int n,int nph,long& jd,double& frac)
{
    int int_part = 0;
    double lunar_cycles = n + ( nph / 4.0 ); // the total number lunar cycle
                                             // nph = 4 is one lunar cycle 
    double t = lunar_cycles / 1236.85;       // 1236.85 is the number of lunar cycle per Julian Century
    double t2 = t * t;                       // Square for frequent use

    // Sun's mean anomaly
    double sun_anomaly = 359.2242 + ( 29.10535608 * lunar_cycles );

    // Moon's mean anomaly
    double moon_anomaly = 306.0253 + ( 385.81691806 * lunar_cycles ) + ( 0.010730 * t2 );

    // Not sure of what's up here, but two of the numbers very interesting
    // Notice that 2415020.75933 is used a referce epochs for mean time of lunar cycle which is 1900 Jan 1 at 6:13 AM
    // And 29.53058868 is the "Synodic month" or the mean time in days between new moons.
    double xtra = 0.75933 + ( 1.53058868 * lunar_cycles ) + ( ( ( 1.78e-4 ) - ( 1.55e-7 ) * t ) * t2 );

    // Ok 2415020 is Julian date 1899 at noon + .75933 is the Julian date of a new moon
    // 28 + 1.53058868 is mean time of a lunar cycle ,
    // and 7 close to 7.38264717 a lunar phase but unlike
    // the first two the tractional part dose not seem to be accounted for. 
    jd = 2415020 + ( 28L * n ) + ( 7L * nph );

    // Looks like this is all being done to adjust the variations in the maen lunar cycle of 29.53058868 
    // Which from what I understand is due the moons orbit in relationship to the earths orbit around the sun. 
    if(nph == 0 || nph == 2) // New or Full - sun earth moon inline
        xtra += ( ( 0.1734-3.93e-4 * t ) * sin(RAD*sun_anomaly) ) - ( 0.4068 * sin(RAD*moon_anomaly) );
    else // 1st quarder last quarter - moon 90 deegree left or right of the earth and suns line 
        xtra += ( ( 0.1721-4.0e-4 * t ) * sin(RAD*sun_anomaly) ) - ( 0.6280 * sin(RAD*moon_anomaly) );

    // This parts easy just put the integer and fractional parts right
    int_part = (int)( xtra >= 0.0 ? floor(xtra) : ceil(xtra-1.0) );
    jd += int_part;
    frac = xtra - int_part;
    return double(jd + frac);

} // End of flmoon()


///////////////////////////////////////////////////////////////

void Moon::Julian2Calendar(long jd,int& mon,int& day,int& year)
{
    const long GREG = 2299161L;
    long a = 0L;
    long b = 0L;
    long c = 0L;
    long d = 0L;
    long e = 0L;

    if(jd >= GREG) {
        a = (long)((((jd-1867216)-0.25)/36524.25));
        a = (long)(jd+1+(long)a-(long)(0.25*(long)a));
    } else {
        a = jd;
    }
    b = a+1524;
    c = (long)(6680+((b-2439870)-122.1)/365.25);
    d = 365*c+(long)(0.25*c);
    e = (long)((b-d)/30.6001);
    day = (int)(b-d-(long)(30.6001*e));
    mon = (int)(e-1);
    if(mon > 12) mon -= 12;
    year = (int)(c-4715);
    if(mon > 2) --year;
    if(year <= 0) --year;

} // End of Julian2Calendar()

///////////////////////////////////////////////////////////////

long Moon::Calendar2Julian(int mon,int day,int year)
{
    long jul;
    int ja,jy=year,jm;

    if(jy == 0) return(0);
    if(jy < 0) ++jy;
    if(mon > 2) jm=mon+1;
    else {
        --jy;
        jm=mon+13;
    }
    jul = (long)(floor(365.25*jy)+floor(30.6001*jm)+day+1720995);
    if(day+31L*(mon+12L*year) >= IGREG) {
        ja=(int)(0.01*jy);
        jul += 2-ja+(int)(0.25*ja);
    }
    return(jul);

} // End of Calendar2Julian()

///////////////////////////////////////////////////////////////

bool Moon::isDST(long jd)
{
    int m,d,y;

    Julian2Calendar(jd,m,d,y);
    return(isDST(m,d,y));

} // End of isDST()

///////////////////////////////////////////////////////////////

bool Moon::isDST(int mon,int day,int year)
{
    struct tm tmp;
    struct tm *tmpt;
    time_t e;

    e = time(NULL);
    tmpt = localtime(&e);
    // tmp.tm_gmtoff = tmpt->tm_gmtoff;
    tmp.tm_mon = mon - 1;
    tmp.tm_mday = day;
    tmp.tm_year = year - 1900;
    tmp.tm_sec = 0;
    tmp.tm_min = 0;
    tmp.tm_hour = 3;
    tmp.tm_isdst = -1;
    e = mktime(&tmp);
    tmpt = localtime(&e);
    if(tmpt->tm_isdst) return(true);
    return(false);

} // End of isDST()

///////////////////////////////////////////////////////////////
///////////////////// END OF FILE /////////////////////////////
///////////////////////////////////////////////////////////////
