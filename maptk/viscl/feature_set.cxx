/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */


#include <maptk/viscl/feature_set.h>
#include <viscl/core/manager.h>

#include "utils.h"

namespace maptk
{

namespace vcl
{


/// Return a vector of feature shared pointers
std::vector<feature_sptr>
feature_set
::features() const
{
  std::vector<feature_sptr> features;

  int numkpts = size();
  viscl::cl_queue_t queue = viscl::manager::inst()->create_queue();

  std::vector<cl_int2> kpts;
  kpts.resize(numkpts);
  queue->enqueueReadBuffer(*data_.features_().get(), CL_TRUE, 0, sizeof(cl_int2)*numkpts, &kpts[0]);

  //These features only have locations set for them
  for (unsigned int i = 0; i < numkpts; i++)
  {
    feature_f *f = new feature_f();
    f->set_loc(vector_2f(kpts[i].s[0], kpts[i].s[1]));
    features.push_back(feature_sptr(f));
  }

  return features;
}


// Convert any feature set to a VisCL data (upload if needed)
feature_set::type
features_to_viscl(const maptk::feature_set& features)
{
  //if already on GPU in VisCL format, then access it
  if( const vcl::feature_set* f =
          dynamic_cast<const vcl::feature_set*>(&features) )
  {
    return f->viscl_features();
  }

  unsigned int width, height;
  min_image_dimensions(features, width, height);

  //kpt map is half width and height
  unsigned int ni, nj;
  ni = (width >> 1) + 1;
  nj = (height >> 1) + 1;
  size_t size = ni * nj;
  int *kp_map = new int [size];
  for (unsigned int i = 0; i < size; i++)
    kp_map[i] = -1;

  feature_set::type fs;
  int numfeat[1];
  numfeat[0] = static_cast<int>(features.size());
  fs.features_ = viscl::manager::inst()->create_buffer<cl_int2>(CL_MEM_READ_WRITE, numfeat[0]);
  fs.numfeat_ = viscl::manager::inst()->create_buffer<int>(CL_MEM_READ_WRITE, 1);

  //write number of features
  viscl::cl_queue_t queue = viscl::manager::inst()->create_queue();
  queue->enqueueWriteBuffer(*fs.numfeat_().get(), CL_TRUE, 0, fs.numfeat_.mem_size(), numfeat);

  //write features
  std::vector<feature_sptr> feat = features.features();
  cl_int2 *buf = new cl_int2[features.size()];
  typedef std::vector<feature_sptr>::const_iterator feat_itr;
  unsigned int j = 0;
  for(feat_itr it = feat.begin(); it != feat.end(); ++it, j++)
  {
    const feature_sptr f = *it;

    cl_int2 kp;
    kp.s[0] = static_cast<int>(f->loc()[0]);
    kp.s[1] = static_cast<int>(f->loc()[1]);
    buf[j] = kp;
    int index = (kp.s[0] >> 1) * nj + (kp.s[1] >> 1);
    assert(index  < size);

    kp_map[index] = j;
  }

  queue->enqueueWriteBuffer(*fs.features_().get(), CL_TRUE, 0, fs.features_.mem_size(), buf);
  queue->finish();

  cl::ImageFormat kptimg_fmt(CL_R, CL_SIGNED_INT32);
  fs.kptmap_ = viscl::image(boost::make_shared<cl::Image2D>(
                            viscl::manager::inst()->get_context(),
                            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                            kptimg_fmt,
                            ni,
                            nj,
                            0,
                            (void *)kp_map));

  delete [] buf;
  delete [] kp_map;

  return fs;
}

size_t
feature_set
::size() const
{
  int buf[1];
  viscl::cl_queue_t q = viscl::manager::inst()->create_queue();
  q->enqueueReadBuffer(*data_.numfeat_().get(), CL_TRUE, 0, data_.numfeat_.mem_size(), buf);
  return buf[0];
}


} // end namespace vcl

} // end namespace maptk
