#ifndef LOCATIONSTATUS_H
#define LOCATIONSTATUS_H

enum LocationStatus {
  INVALID=0,
  SCANNING,
  CONNECT,
  REQUESTING,
  UPDATING,
  UPDATED
};

extern LocationStatus locationStatus;

#endif
