
#ifdef HAVE_NODE_NODE_H
#include <node/node.h>
#elif defined(HAVE_NODEJS_DEPS_NODE_NODE_H)
#include <nodejs/deps/node/node.h>
#elif defined(HAVE_NODEJS_DEPS_NODE_INCLUDE_NODE_H)
#include <nodejs/deps/node/include/node.h>
#elif defined(HAVE_NODEJS_SRC_NODE_H)
#include <nodejs/src/node.h>
#elif defined(HAVE_NODE_H)
#include <node.h>
#else
#error We must have at least one node header to include
#endif
