/*
   The MIT License (MIT)

   Copyright (c) 2014 Antonio SJ Musumeci <trapexit@spawn.link>

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

#include <string>
#include <vector>

#include <sys/types.h>
#include <errno.h>
#include <string.h>

#include "config.hpp"
#include "fs.hpp"
#include "ugid.hpp"
#include "assert.hpp"
#include "xattr.hpp"

using std::string;
using std::vector;
using mergerfs::Policy;
using namespace mergerfs;

static
int
_listxattr_controlfile(char         *list,
                       const size_t  size)
{
#ifndef WITHOUT_XATTR
  const char xattrs[] =
    "user.mergerfs.action\0"
    "user.mergerfs.create\0"
    "user.mergerfs.search\0"
    "user.mergerfs.statfs\0";

  if(size == 0)
    return sizeof(xattrs);

  if(size < sizeof(xattrs))
    return -ERANGE;

  memcpy(list,xattrs,sizeof(xattrs));

  return sizeof(xattrs);
#else
  return -ENOTSUP;
#endif
}

static
int
_listxattr(const Policy::Search::Func  searchFunc,
           const vector<string>       &srcmounts,
           const string                fusepath,
           char                       *list,
           const size_t                size)
{
#ifndef WITHOUT_XATTR
  int rv;
  string path;

  path = searchFunc(srcmounts,fusepath).full;
  if(path.empty())
    return -ENOENT;

  rv = ::llistxattr(path.c_str(),list,size);

  return ((rv == -1) ? -errno : rv);
#else
  return -ENOTSUP;
#endif
}

namespace mergerfs
{
  namespace listxattr
  {
    int
    listxattr(const char *fusepath,
              char       *list,
              size_t      size)
    {
      const ugid::SetResetGuard  ugid;
      const config::Config      &config = config::get();

      if(fusepath == config.controlfile)
        return _listxattr_controlfile(list,
                                      size);

      return _listxattr(config.policy.search,
                        config.srcmounts,
                        fusepath,
                        list,
                        size);
    }
  }
}