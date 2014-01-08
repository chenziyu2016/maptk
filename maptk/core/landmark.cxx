/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "landmark.h"


/**
 * \file
 * \brief Implementation and template instantiation for
 * \link maptk::landmark landmark \endlink objects
 */

namespace maptk
{


/// output stream operator for a landmark base class
std::ostream&  operator<<(std::ostream& s, const landmark& m)
{
  // TODO include covariance once stream operators are defined
  s << m.loc() << " "
    << m.scale();
  return s;
}


/// Default Constructor
template <typename T>
landmark_<T>
::landmark_()
: loc_(0,0,0),
  scale_(1)
{
}


/// Constructor for a feature
template <typename T>
landmark_<T>
::landmark_(const vector_3_<T>& loc, T scale)
: loc_(loc),
  scale_(scale)
{
}


/// output stream operator for a landmark
template <typename T>
std::ostream&  operator<<(std::ostream& s, const landmark_<T>& m)
{
  // TODO include covariance once stream operators are defined
  s << m.get_loc() << " "
    << m.get_scale();
  return s;
}


/// input stream operator for a landmark
template <typename T>
std::istream&  operator>>(std::istream& s, landmark_<T>& m)
{
  // TODO include covariance once stream operators are defined
  vector_3_<T> loc;
  T scale;
  s >> loc
    >> scale;
  m.set_loc(loc);
  m.set_scale(scale);
  return s;
}


#define INSTANTIATE_LANDMARK(T) \
template class landmark_<T>; \
template std::ostream&  operator<<(std::ostream& s, const landmark_<T>& f); \
template std::istream&  operator>>(std::istream& s, landmark_<T>& f)

INSTANTIATE_LANDMARK(double);
INSTANTIATE_LANDMARK(float);

#undef INSTANTIATE_LANDMARK

} // end namespace maptk
