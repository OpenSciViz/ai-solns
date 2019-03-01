/** 
 * \file AhTimeElem.h
 * \brief class defining flexible format for time in seconds
 * \author Mike Witthoeft
 * \date $Date: 2012/06/19 21:31:33 $
 *
 * This class defines a lossless format for storing precision timings similar
 * in design to floating point numbers.  Each time (in seconds) is stored with
 * an integer mantissa and a negated binary exponent (as opposed to the base-10
 * exponent of floating point numbers).  The format is: m x 2^(-e) where m is a
 * 64-bit unsigned integer (current specification using the AteMantissa typedef)
 * and e is 32-bit integer (typedef of AteExponent).  The range of e is
 * restricted to non-negative integers.
 * 
 */
 
#ifndef AHTIME_AHTIMEELEM_H_
#define AHTIME_AHTIMEELEM_H_

/// \ingroup mod_ahtime
namespace ahtime {

  /** \addtogroup mod_ahtime
   *  @{
   */

  /// \brief type for AhTimeElem mantissa
  typedef unsigned long long AteMantissa;


  /// \brief type for AhTimeElem binary exponent
  typedef unsigned int AteExponent;


  /// \brief store a time in seconds as a mantissa and binary exponent
  ///
  /// This class stores a time in seconds as a mantissa (type AteMantissa) and
  /// a binary exponent (type AteExponent).  The time is reconstructed using 
  /// m x 2^e where m is the mantissa and e is the exponent.  With the current
  /// type of AteMantissa (unsigned long long), this allows for 64 binary bits
  /// of precision for the time which can represent decades of time down to
  /// units of microseconds.
  ///
  /// \internal
  /// \todo overload addition operator for add() and subtract()
  class AhTimeElem {
    private:
      AteMantissa m_mant;          ///< mantissa 
      AteExponent m_expo;          ///< binary exponent

    public:

      /// \brief constructor giving mantissa and binary exponent
      /// \param[in] mantissa number of time units
      /// \param[in] exponent with units of 1/2^x, x is the scale
      AhTimeElem(AteMantissa mantissa, AteExponent exponent);

      /// \brief constructor giving seconds and length of mantissa
      /// \param[in] time number of seconds
      /// \param[in] mlen minimum length of binary mantissa (default: 5)
      AhTimeElem(double time,int mlen=5);

      /// \brief return time mantissa
      /// \return mantissa
      AteMantissa mantissa() const;

      /// \brief return binary exponent
      /// \return exponent
      AteExponent exponent() const;

      /// \brief change the exponent
      /// \param[in] newexpo desired exponent
      /// \return true if successful, false otherwise
      ///
      /// This function will attempt to change the scale of the time quantity.
      /// If the newexpo is smaller (coarser) than the original scale, then
      /// some time information will be lost.  If the scale is larger (finer),
      /// the function will check to see if the new quantity can fit into
      /// 64-bits (long long) before conversion.
      bool rescale(AteExponent newexpo);

      /// \brief add to current time and return result
      /// \param[in] x quantity to add 
      /// \return sum of times
      ///
      /// Add two times together and return result.  Scale is adjusted to the
      /// finer resolution prior to addition.
      AhTimeElem add(AhTimeElem & x);

      /// \brief subtract value from current time and return result
      /// \param[in] x quantity to subtract
      /// \return sum of times
      ///
      /// Subtract given time from object and return result.  The result must
      /// be a positive number.  Scale is adjusted to the finer resolution 
      /// prior to subtraction.
      AhTimeElem subtract(AhTimeElem & x);

      /// \brief make the scale as small (coarse) as possible without changing
      ///        the time value
      ///
      /// Reduce (coarsen) the time scale as much as possible without changing
      /// the time value.  This can increase the maximum value possible since
      /// it frees up bits.
      void minimizeExponent();


      /// \brief convert the time to a double, possibly losing information
      /// \return time in seconds
      ///
      /// \internal
      /// \todo check if conversion method is accurate
      double seconds() const;


  };

  /** @} */

}

#endif /* AHTIME_AHTIMEELEM_H_ */

/* Revision Log
 $Log: AhTimeElem.h,v $
 Revision 1.1  2012/06/19 21:31:33  mwitthoe
 move core/lib/ahtimeconv to core/lib/ahtime

 Revision 1.3  2012/04/24 19:51:31  mwitthoe
 remove bugs in ahtime

 Revision 1.2  2012/04/24 19:16:18  mwitthoe
 documentation for ahtime


*/
