#include <iostream>
#include <stdexcept>
#include <math.h>
#include "ahtime/AhTimeElem.h"

namespace ahtime {


  /// \callgraph
  AhTimeElem::AhTimeElem(AteMantissa mantissa, AteExponent exponent) {
    m_mant=mantissa;
    m_expo=exponent;
  }


  /// \callgraph
  AhTimeElem::AhTimeElem(double time, int mlen) {

    // time must be positive
    if (time < 0.0) {
      throw std::runtime_error("cannot initialize with a negative time");
    }

    // zero
    if (time == 0.0) {
      m_mant=0;
      m_expo=0;
      return;
    }

    // split into integral and fractional parts
    AteMantissa xint=(AteMantissa)floor(time);
    double xfrac=time-(double)xint;

    // deal with integral part first
    int nint=0;         // number of integral digits
    if (xint > 0) nint=1+(int)(log(xint)/log(2.));
    /// \internal \todo should there be a range check?
    if (nint >= mlen || xfrac == 0.0) {
      m_mant=xint;
      m_expo=0;
      return;
    }

    // fractional part
    int nleft=mlen-nint;
    bool startcount=false;
    if (nleft < mlen) startcount=true;
    int nloop=0;
    int count=0;
    int val=0;
    while (count < nleft) {
      nloop++;
      val=val<<1;
      xfrac*=2;
      if (xfrac >= 1.0) {
        val++;
        xfrac=xfrac-floor(xfrac);
        startcount=true;
      }
      if (startcount) count++;
    }
    m_mant=(xint<<nleft)+val;
    m_expo=(AteExponent)nloop;
  }

  /// \callgraph
  AteMantissa AhTimeElem::mantissa() const {
    return m_mant;
  }


  /// \callgraph
  AteExponent AhTimeElem::exponent() const {
    return m_expo;
  }


  /// \callgraph
  bool AhTimeElem::rescale(AteExponent newexpo) {
    if (newexpo == m_expo) {
      return true;
    } else if (newexpo > m_expo) {
      // check if number with fit into 64-bits with new exponent
      int ndigits=1+(int)(log((double)m_mant)/log(2.0));
      if (ndigits-m_expo+newexpo > 63) return false;

      m_mant=m_mant << (newexpo-m_expo);
    } else {
      m_mant=m_mant >> (m_expo-newexpo);
    }
    m_expo=newexpo;
    return true;
  }


  /// \callgraph
  AhTimeElem AhTimeElem::add(AhTimeElem & x) {
    // make exponents consistent, choosing finest scale
    AteExponent expo=std::max(m_expo,x.exponent());
    rescale(expo);
    x.rescale(expo);

    return AhTimeElem(m_mant+x.mantissa(),expo);
  }


  /// \callgraph
  AhTimeElem AhTimeElem::subtract(AhTimeElem & x) {
    // make exponents consistent, choosing finest scale
    AteExponent expo=std::max(m_expo,x.exponent());
    rescale(expo);
    x.rescale(expo);

    // must give positive result
    if (m_mant < x.mantissa()) {
      throw std::runtime_error("subtraction must give positive result");
    }

    return AhTimeElem(m_mant-x.mantissa(),expo);
  }


  /// \callgraph
  void AhTimeElem::minimizeExponent() {
    if (m_expo <= 0) return;
    while (m_mant == ( (m_mant >> 1) << 1)) {
      m_mant=m_mant >> 1;
      m_expo--;
      if (m_expo == 0) break;
    }
  }


  /// \callgraph
  /// \internal
  /// \note original implementation was out=double(m_mant)*pow(2,-m_expo), but
  /// this fails with a large exponent, so do it the hard way...
  double AhTimeElem::seconds() const {
    double out=(double)m_mant;
    for (int i=0; i<m_expo; i++) out*=0.5;
    return out;
  }

}
