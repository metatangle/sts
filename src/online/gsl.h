#ifndef STS_GSL_H
#define STS_GSL_H

#include <functional>
#include <gsl/gsl_min.h>

// The default GSL error handler aborts on errors; this one throws exceptions instead.
extern "C" void sts_gsl_error_handler(const char* reason, const char* file, int line, int gsl_errno);

namespace sts
{
namespace gsl
{
double minimize(const std::function<double(double)> fn,
                double m = 0.5,
                double a = 0,
                double b = 1,
                const int max_iter = 100,
                const gsl_min_fminimizer_type *min_type = gsl_min_fminimizer_brent);
} // namespace gsl
} // namespace sts

#endif // STS_GSL_H
