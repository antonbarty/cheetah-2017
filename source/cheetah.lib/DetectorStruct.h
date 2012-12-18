#ifndef DETECTORSTRUCT_H
#define DETECTORSTRUCT_H



typedef struct{
    hid_t detector_gid;
    hid_t distance_did;
    hid_t data_did;
    hid_t snapshot_did;
}Detector;


typedef struct{
    hid_t image_gid;
    hid_t data_did;
    hid_t snapshot_did;
}Image;


#endif // DETECTORSTRUCT_H
