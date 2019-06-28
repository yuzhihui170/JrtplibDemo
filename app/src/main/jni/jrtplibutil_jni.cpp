#include <jni.h>
#include <android/native_window_jni.h>

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <JRTPLIB/src/rtpipv4address.h>
#include <JRTPLIB/src/rtptimeutilities.h>
#include <JRTPLIB/src/rtpudpv4transmitter.h>
#include <JRTPLIB/src/rtpsessionparams.h>
#include <JRTPLIB/src/rtpsession.h>
#include <JRTPLIB/src/rtppacket.h>

#include "rtpsession.h"
#include "rtpudpv4transmitter.h"
#include "rtpipv4address.h"
#include "rtpsessionparams.h"
#include "rtperrors.h"
#include "rtplibraryversion.h"

#include "fflog.h"

using namespace jrtplib;

// This function checks if there was a RTP error. If so, it displays an error message and exists.
#define CHECK_ERROR_JRTPLIB(status) \
    if (status < 0) { \
        LOGFE("ERROR: %s", jrtplib::RTPGetErrorString(status).c_str()); \
        exit(-1);\
    }

int test1() {
    RTPSession sess;
    uint16_t portbase, destport;
    uint32_t destip;
    std::string ipstr;
    int status, i, num;

    LOGFD("Using version %s", RTPLibraryVersion::GetVersion().GetVersionString().c_str());

    // First, we'll ask for the necessary information
    portbase = 6000;

    ipstr = "127.0.0.1";
    destip = inet_addr(ipstr.c_str());
    if (destip == INADDR_NONE) {
        LOGFE("Bad IP address specified");
        return -1;
    }

    // The inet_addr function returns a value in network byte order, but
    // we need the IP address in host byte order, so we use a call to
    // ntohl
    destip = ntohl(destip);
    destport = 6000;

    num = 2;

    // Now, we'll create a RTP session, set the destination, send some packets and poll for incoming data.
    RTPUDPv4TransmissionParams transparams;
    RTPSessionParams sessparams;

    // IMPORTANT: The local timestamp unit MUST be set, otherwise RTCP Sender Report info will be calculated wrong
    // In this case, we'll be sending 10 samples each second, so we'll put the timestamp unit to (1.0/10.0)
    sessparams.SetOwnTimestampUnit(1.0 / 10.0);

    sessparams.SetAcceptOwnPackets(true);
    transparams.SetPortbase(portbase);
    status = sess.Create(sessparams, &transparams);
    CHECK_ERROR_JRTPLIB(status);

    RTPIPv4Address addr(destip, destport);

    status = sess.AddDestination(addr);
    CHECK_ERROR_JRTPLIB(status);

    status = sess.SendPacket((void *) "1234567890", 10, 0, false, 10);
    CHECK_ERROR_JRTPLIB(status);

    status = sess.SendPacket((void *) "0987654321", 10, 0, false, 10);
    CHECK_ERROR_JRTPLIB(status);

    for (i = 1; i <= num; i++) {
        sess.BeginDataAccess();

        // check incoming packets
        if (sess.GotoFirstSourceWithData()) {
            do {
                RTPPacket *pack;
                while ((pack = sess.GetNextPacket()) != NULL) {
                    // You can examine the data here
                    printf("Got packet !\n");
                    // we don't longer need the packet, so we'll delete it
                    sess.DeletePacket(pack);
                }
            } while (sess.GotoNextSourceWithData());
        }

        sess.EndDataAccess();

#ifndef RTP_SUPPORT_THREAD
        status = sess.Poll();
        checkerror(status);
#endif // RTP_SUPPORT_THREAD

        RTPTime::Wait(RTPTime(1, 0));
    }

    sess.BYEDestroy(RTPTime(10, 0), 0, 0);
    return 1;
}


extern "C"
JNIEXPORT void JNICALL
Java_com_forrest_jrtplib_JrtplibUtil_test(JNIEnv *env, jobject instance) {
    test1();
}