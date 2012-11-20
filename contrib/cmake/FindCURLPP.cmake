FIND_PATH(CURLPP_INCLUDE_DIR curlpp/cURLpp.hpp
  PATH_SUFFIXES include
  PATHS
  /usr/local
  /usr
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
  HINTS $ENV{CURLPP_SRC}
)

FIND_LIBRARY(CURLPP_LIBRARY
  NAMES curlpp
  HINTS $ENV{CURLPP_SRC}/.libs/
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CURLPP DEFAULT_MSG CURLPP_LIBRARY CURLPP_INCLUDE_DIR)
MARK_AS_ADVANCED(CURLPP_LIBRARY CURLPP_INCLUDE_DIR)