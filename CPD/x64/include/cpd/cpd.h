#ifndef _CPD_HEADER_FILES_H_
#define _CPD_HEADER_FILES_H_

#include <cpd/version.hpp>
#include <cpd/utils.hpp>
#include <cpd/matrix.hpp>
#include <cpd/normalization.hpp>
#include <cpd/gauss_transform.hpp>
#include <cpd/transform.hpp>
#include <cpd/rigid.hpp>
#include <cpd/affine.hpp>
#include <cpd/nonrigid.hpp>

#ifdef _DEBUG
#pragma comment(lib,"cpd_d.lib")
#else
#pragma comment(lib,"cpd.lib")
#endif


#endif //!_CPD_HEADER_FILES_H_