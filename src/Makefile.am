bin_PROGRAMS = kal

kal_SOURCES = \
   arfcn_freq.cc \
   c0_detect.cc	 \
   circular_buffer.cc \
   fcch_detector.cc \
   kal.cc \
   offset.cc \
   lime_source.cc \
   util.cc\
   arfcn_freq.h \
   c0_detect.h \
   circular_buffer.h \
   fcch_detector.h \
   offset.h \
   complex.h \
   lime_source.h \
   util.h\
   version.h

kal_CXXFLAGS = $(FFTW3_CFLAGS) $(LMS_CFLAGS) -std=c++11
kal_LDADD = $(FFTW3_LIBS) $(LMS_LIBS)
