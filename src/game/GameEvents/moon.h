/////////////////////////////////////////////////////////////////////////
///
/// moon.h
///
///
/// Created March 31, 2004 by Don Dugger
///
///
///
/////////////////////////////////////////////////////////////////////////
///
/// <PRE>
/// THE SOFTWARE IS PROVIDED ~AS IS~, WITHOUT WARRANTY OF ANY KIND,
/// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
/// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
/// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
/// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
/// DEALINGS IN THE SOFTWARE.
/// </PRE>
///
/////////////////////////////////////////////////////////////////////////

#ifndef _moon_h_
#define _moon_h_

/////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/////////////////////////////////////////////////////////////////////////
/// <STRONG>
/// Classes
/// </STRONG>
/// <p>
///
///   This class is an all static.
/// </p><p>
///   This group of metthods deals with the phases of the moon, it also computes<BR>
///   Julian dates, due to the fact it was needed for the moon calculations. To make it<BR>
///   simpler to work with the Julian calculation there left in rather than creating a<BR>
///   separate class.
/// </p><p>
///   Some functions are taken from "Numerical Recipes in C - The Art of Scientific Computing"
///   Second Edition, tranlated with changes for C++ and help clarify the code.
/// <BR>
///   ISBN 0-521-43108-5
/// <BR>
///   (Copyright &copy; Cambridge University Press 1988, 1992)
/// </p><p>
///   \section lunarphase  Lunar Phases
/// </p><p>
///    The program definds the phase of the moon as ia floating point number between 0 and 4.
///    The following values apply:
///    - 0.0 = New Moon
///    - 1.0 = 1st Quarter
///    - 2.0 = Full Moon
///    - 3.0 = Last Quarter
/// </p>
///@brief Calculate the phase of the moon.
///
/////////////////////////////////////////////////////////////////////////
class Moon {
    /// Radians to degree convertion
    static const double RAD;
    /// We will call this the Gregorian Interval.
    /// The Gregorian Calendar was adopted in October 15, 1582.
    static const long IGREG;
    // The mean lunar cycle
    static const double MEAN_LUNAR_CYCLE;

public:

    Moon() {};
    ~Moon() {};

    /////////////////////////////////////////////////////////////////////////
    /// phase() calculates the phase of the moon at
    /// noon of the date given.
    ///<p> <i>See above</i> @ref lunarphase
    ///@param mon The month.
    ///@param day The day of the month.
    ///@param year The year.
    ///@return  The Lunar Phase <i>see above</i> \ref lunarphase
    static double phase(int mon,int day,int year);

    /////////////////////////////////////////////////////////////////////////
    /// nextphase() calculates the date & time of next phase of the moon,
    /// that is the next hole number, given the start date.
    ///@param mon   The month.
    ///@param day   The day of the month.
    ///@param year  The year.
    ///@param hr    The hour.
    ///@param min   The minute.
    ///@param nph   The Lunar Phase <i>See above</i> \ref lunarphase
    ///@return  Note that all the return values are references.
    static void nextphase(int& mon,int& day,int& year,int& hr,int& min,int nph);

    /////////////////////////////////////////////////////////////////////////
    /// flmoon() calculates the julian date of nth lunar cycle and nph phase
    /// of the moon since Jan 1900
    /// n is the number of lunar cycle since Jan 1900 - ???
    /// This is the original function from the book.
    /// It's been modified to work in C++.
    /// I also rewrote it to be litte clearer. And added
    /// comments where I unterstood what was going on and 
    /// increased the accuracy of a few constants.
    ///@param n     The number of lunar cycles.
    ///@param nph   The lunar phase.
    ///@param jd    The Julian date number.
    ///@param frac  The tractional part of the Julian date number.
    ///@return      The full Julian date of the nth lunar cycle plus nph phase
    ///             Note that the integer and fractional parts are
    ///             returned by references.
    static double flmoon(int n,int nph,long& jd,double& frac);

    /////////////////////////////////////////////////////////////////////////
    /// Julian2Calendar() computes the julian date
    ///@param jd    The Julian date number.
    ///@param mon   The month.
    ///@param day   The day of the month.
    ///@param year  The year.
    ///@return  Note that all the return values are references.
    static void Julian2Calendar(long jd,int& mon,int& day,int& year);

    /////////////////////////////////////////////////////////////////////////
    /// Calendar2Julian() computes the date given a julian date
    ///@param mon   The month.
    ///@param day   The day of the month.
    ///@param year  The year.
    static long Calendar2Julian(int mon,int day,int year);

    /////////////////////////////////////////////////////////////////////////
    /// isDST() returns true if the given date is day light savings time
    ///     There are two versions, one which takes the Julian date and
    ///     one that takes the monuth, day and year.
    ///@param   jd  The Julian date
    static bool isDST(long jd);
    /////////////////////////////////////////////////////////////////////////
    ///@param mon   The month.
    ///@param day   The day of the month.
    ///@param year  The year.
    static bool isDST(int mon,int day,int year);
    
};


/////////////////////////////////////////////////////////////////////////

#endif /// _moon_h_

