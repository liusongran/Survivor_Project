#include <task.h>

#include <apps.h>
#include <./src/apps/Lib/accel.h>
#include <./src/apps/Lib/mspmath/msp-math.h>

unsigned _v_seed;

// Number of samples to discard before recording training set
#define NUM_WARMUP_SAMPLES 6
#define ACCEL_WINDOW_SIZE 24
#define MODEL_SIZE 64
#define SAMPLE_NOISE_FLOOR 10 // TODO: made up value

// Number of classifications to complete in one experiment
#define SAMPLES_TO_COLLECT 96

typedef threeAxis_t_8 accelReading;
typedef accelReading accelWindow[ACCEL_WINDOW_SIZE];

typedef struct {
    unsigned meanmag;
    unsigned stddevmag;
} features_t;

typedef enum {
    CLASS_STATIONARY,
    CLASS_MOVING,
} class_t;

typedef enum {
    MODE_IDLE = 3,
    MODE_TRAIN_STATIONARY = 2,
    MODE_TRAIN_MOVING = 1,
    MODE_RECOGNIZE = 0, // default
} run_mode_t;


/**
 * 1. TASK declaration here.
 */
TASK(task_init);            //-(0)
TASK(task_selectMode);      //-(1)
TASK(task_sample);          //-(3)
//TASK(task_featurize);       //-(5)
//TASK(task_classify);        //-(6)
//TASK(task_train);           //-(9)
TASK(task_idle);            //-(10)

/**
 * 2. Shared variable declaration here. (164 bytes)
 */
__shared(
    uint16_t _v_pinState;                       //-[1]:2
    unsigned _v_discardedSamplesCount;          //-[2]:2
    class_t _v_class;                           //-[3]:
    unsigned _v_totalCount;                     //-[4]:2
    unsigned _v_movingCount;                    //-[5]:2
    unsigned _v_stationaryCount;                //-[6]:2
    accelReading _v_window[ACCEL_WINDOW_SIZE];  //-[7]:72
    features_t _v_features;                     //-[8]:4
    unsigned _v_trainingSetSize;                //-[9]:2
    unsigned _v_samplesInWindow;                //-[10]:2
    run_mode_t _v_mode;                         //-[11]:4
    unsigned _v_seed;                           //-[12]:2
    unsigned _v_count;                          //-[13]:2 ----98
    features_t _v_model_stationary[MODEL_SIZE]; //-[14]:32*4 = 128  64*4 = 256  ----354
    features_t _v_model_moving[MODEL_SIZE];     //-[15]:32*4 = 128  64*4 = 256  ----610
)

//MIN
// redVerSum 234
// redBakCksumSum 2144
// Solution [13.  1.  8.  7. 12.  3.  9. 11. 10. 14. 15.  5.  2.  6.  4.]
// [[0, 81], [0, 353], [4, 345], [0, 353]]

//__shared(
//    unsigned _v_count;                          //-[13]:2
//    uint16_t _v_pinState;                       //-[1]:2
//    features_t _v_features;                     //-[8]:4
//    accelReading _v_window[ACCEL_WINDOW_SIZE];  //-[7]:12
//    unsigned _v_seed;                           //-[12]:2
//    class_t _v_class;                           //-[3]:
//    unsigned _v_trainingSetSize;                //-[9]:2
//    run_mode_t _v_mode;                         //-[11]:4
//    unsigned _v_samplesInWindow;                //-[10]:2
//    features_t _v_model_stationary[MODEL_SIZE]; //-[14]:64
//    features_t _v_model_moving[MODEL_SIZE];     //-[15]:64
//    unsigned _v_movingCount;                    //-[5]:2
//    unsigned _v_discardedSamplesCount;          //-[2]:2
//    unsigned _v_stationaryCount;                //-[6]:2
//    unsigned _v_totalCount;                     //-[4]:2
//)

//MAX
// redVerSum 20756
// redBakCksumSum 125436
// Solution [12.  5. 14.  4. 13.  9.  7. 11.  6.  8.  2. 15.  1. 10.  3.]
// [[0, 351], [0, 353], [0, 353], [132, 135]]

// __shared(
//     unsigned _v_seed;                           //-[12]:2
//     unsigned _v_movingCount;                    //-[5]:2
//     features_t _v_model_stationary[MODEL_SIZE]; //-[14]:64
//     unsigned _v_totalCount;                     //-[4]:2
//     unsigned _v_count;                          //-[13]:2
//     unsigned _v_trainingSetSize;                //-[9]:2
//     accelReading _v_window[ACCEL_WINDOW_SIZE];  //-[7]:12
//     run_mode_t _v_mode;                         //-[11]:4
//     unsigned _v_stationaryCount;                //-[6]:2
//     features_t _v_features;                     //-[8]:4
//     unsigned _v_discardedSamplesCount;          //-[2]:2
//     features_t _v_model_moving[MODEL_SIZE];     //-[15]:64
//     uint16_t _v_pinState;                       //-[1]:2
//     unsigned _v_samplesInWindow;                //-[10]:2
//     class_t _v_class;                           //-[3]:
// )

//MAX2
//redVerSum 20756
//redBakCksumSum 125436
//Solution [12.  4. 14.  3. 11.  5.  7.  8.  6. 13. 10. 15.  2.  1.  9.]
//__shared(
//    unsigned _v_seed;                           //-[12]:2
//    unsigned _v_totalCount;                     //-[4]:2
//    features_t _v_model_stationary[MODEL_SIZE]; //-[14]:64
//    class_t _v_class;                           //-[3]:
//    run_mode_t _v_mode;                         //-[11]:4
//    unsigned _v_movingCount;                    //-[5]:2
//    accelReading _v_window[ACCEL_WINDOW_SIZE];  //-[7]:12
//    features_t _v_features;                     //-[8]:4
//    unsigned _v_stationaryCount;                //-[6]:2
//    unsigned _v_count;                          //-[13]:2
//    unsigned _v_samplesInWindow;                //-[10]:2
//    features_t _v_model_moving[MODEL_SIZE];     //-[15]:64
//    unsigned _v_discardedSamplesCount;          //-[2]:2
//    uint16_t _v_pinState;                       //-[1]:2
//    unsigned _v_trainingSetSize;                //-[9]:2
//)

//MIN-interval
// redAcessSum 4032
// IntervalNum 2
// totalResult 14.569060773480663
//Solution [12.  7.  2.  5.  9.  1. 14. 15. 11.  6.  8. 10.  4. 13.  3.]
//__shared(
//    unsigned _v_seed;                           //-[12]:2
//    accelReading _v_window[ACCEL_WINDOW_SIZE];  //-[7]:12
//    unsigned _v_discardedSamplesCount;          //-[2]:2
//    unsigned _v_movingCount;                    //-[5]:2
//    unsigned _v_trainingSetSize;                //-[9]:2
//    uint16_t _v_pinState;                       //-[1]:2
//    features_t _v_model_stationary[MODEL_SIZE]; //-[14]:64
//    features_t _v_model_moving[MODEL_SIZE];     //-[15]:64
//    run_mode_t _v_mode;                         //-[11]:4
//    unsigned _v_stationaryCount;                //-[6]:2
//    features_t _v_features;                     //-[8]:4
//    unsigned _v_samplesInWindow;                //-[10]:2
//    unsigned _v_totalCount;                     //-[4]:2
//    unsigned _v_count;                          //-[13]:2
//    class_t _v_class;                           //-[3]:
//)

//MAX-interval
//Solution [ 5.  9. 12. 13. 11. 14. 10.  3. 15.  7.  2.  1.  4.  6.  8.]
//__shared(
//    unsigned _v_movingCount;                    //-[5]:2
//    unsigned _v_trainingSetSize;                //-[9]:2
//    unsigned _v_seed;                           //-[12]:2
//    unsigned _v_count;                          //-[13]:2
//    run_mode_t _v_mode;                         //-[11]:4
//    features_t _v_model_stationary[MODEL_SIZE]; //-[14]:64
//    unsigned _v_samplesInWindow;                //-[10]:2
//    class_t _v_class;                           //-[3]:
//    features_t _v_model_moving[MODEL_SIZE];     //-[15]:64
//    accelReading _v_window[ACCEL_WINDOW_SIZE];  //-[7]:12
//    unsigned _v_discardedSamplesCount;          //-[2]:2
//    uint16_t _v_pinState;                       //-[1]:2
//    unsigned _v_totalCount;                     //-[4]:2
//    unsigned _v_stationaryCount;                //-[6]:2
//    features_t _v_features;                     //-[8]:4
//)
/**
 * 3. TASK definition here.
 */
TASK(task_init) { //-->0, NOTE: R() || W(1,12,13) 8us
    __SET(_v_pinState) = MODE_IDLE;
    __SET(_v_count) = 0;
    __SET(_v_seed) = 1;
    __delay_cycles(100);
    NEXT(1);
}

TASK(task_selectMode) { //-->1, NOTE: R(1,13) || W(1,2,3,10,11,13)    10us
    uint16_t pin_state = 1;
    ++__SET(_v_count);

    if (__GET(_v_count) >= 2) pin_state = 2;
    if (__GET(_v_count) >= 3) pin_state = 0;

    // Don't re-launch training after finishing training
    if ((pin_state == MODE_TRAIN_STATIONARY || pin_state == MODE_TRAIN_MOVING) && pin_state == __GET(_v_pinState)) {
        pin_state = MODE_IDLE;
    } else {
        __SET(_v_pinState) = pin_state;
    }

    threeAxis_t_8 sample;
    unsigned seed = __GET(_v_seed);
    switch (pin_state) {
    case MODE_TRAIN_STATIONARY: //2
        __SET(_v_discardedSamplesCount) = 0;
        __SET(_v_mode) = MODE_TRAIN_STATIONARY;
        __SET(_v_class) = CLASS_STATIONARY;
        __SET(_v_samplesInWindow) = 0;
        while (1) {
            if (__GET(_v_discardedSamplesCount) < NUM_WARMUP_SAMPLES) {
                sample.x = (seed * 17) % 85;
                sample.y = (seed * 17 * 17) % 85;
                sample.z = (seed * 17 * 17 * 17) % 85;
                __SET(_v_seed) = ++seed;
                ++__SET(_v_discardedSamplesCount);
            } else {
                __SET(_v_trainingSetSize) = 0;
                break;
            }
        }
        NEXT(2);

    case MODE_TRAIN_MOVING: //1
        __SET(_v_discardedSamplesCount) = 0;
        __SET(_v_mode) = MODE_TRAIN_MOVING;
        __SET(_v_class) = CLASS_MOVING;
        __SET(_v_samplesInWindow) = 0;
        while (1) {
            if (__GET(_v_discardedSamplesCount) < NUM_WARMUP_SAMPLES) {
                sample.x = (seed * 17) % 85;
                sample.y = (seed * 17 * 17) % 85;
                sample.z = (seed * 17 * 17 * 17) % 85;
                __SET(_v_seed) = ++seed;
                ++__SET(_v_discardedSamplesCount);
            } else {
                __SET(_v_trainingSetSize) = 0;
                break;
            }
        }
        NEXT(2);

    case MODE_RECOGNIZE:    //0
        __SET(_v_mode) = MODE_RECOGNIZE;
        __SET(_v_movingCount) = 0;
        __SET(_v_stationaryCount) = 0;
        __SET(_v_totalCount) = 0;
        __SET(_v_samplesInWindow) = 0;
        NEXT(2);

    default:
        NEXT(3);
    }
}



TASK(task_sample) { //-->2, NOTE: R(10,12) || W(7,10,12)  40us~45us
    accelReading sample;
    unsigned i;

    while (1) {
        if (__GET(_v_samplesInWindow) < ACCEL_WINDOW_SIZE) {
            unsigned seed = __GET(_v_seed);
            sample.x = (seed * 17) % 85;
            sample.y = (seed * 17 * 17) % 85;
            sample.z = (seed * 17 * 17 * 17) % 85;
            __SET(_v_seed) = ++seed;
            __SET(_v_window[__GET(_v_samplesInWindow)].x) = sample.x;
            __SET(_v_window[__GET(_v_samplesInWindow)].y) = sample.y;
            __SET(_v_window[__GET(_v_samplesInWindow)].z) = sample.z;
            ++__SET(_v_samplesInWindow);
        } else {
            for (i = 0; i < ACCEL_WINDOW_SIZE; i++) {
                if (__GET(_v_window[i].x) < SAMPLE_NOISE_FLOOR ||
                        __GET(_v_window[i].y) < SAMPLE_NOISE_FLOOR ||
                        __GET(_v_window[i].z) < SAMPLE_NOISE_FLOOR) {

                    __SET(_v_window[i].x) = (__GET(_v_window[i].x) > SAMPLE_NOISE_FLOOR) ? __GET(_v_window[i].x) : 0;
                    __SET(_v_window[i].y) = (__GET(_v_window[i].y) > SAMPLE_NOISE_FLOOR) ? __GET(_v_window[i].y) : 0;
                    __SET(_v_window[i].z) = (__GET(_v_window[i].z) > SAMPLE_NOISE_FLOOR) ? __GET(_v_window[i].z) : 0;
                }
            }
            break;
        }
    }
    //NEXT(3);
    accelReading mean, stddev;
    mean.x = mean.y = mean.z = 0;
    stddev.x = stddev.y = stddev.z = 0;
    features_t features;

    //int i;
    for (i = 0; i < ACCEL_WINDOW_SIZE; i++) {
        mean.x += __GET(_v_window[i].x);
        mean.y += __GET(_v_window[i].y);
        mean.z += __GET(_v_window[i].z);
    }
    mean.x >>= 2;
    mean.y >>= 2;
    mean.z >>= 2;

    //accelReading sample;

    for (i = 0; i < ACCEL_WINDOW_SIZE; i++) {
        sample.x = __GET(_v_window[i].x);
        sample.y = __GET(_v_window[i].y);
        sample.z = __GET(_v_window[i].z);

        stddev.x += (sample.x > mean.x) ? (sample.x - mean.x)
                    : (mean.x - sample.x);
        stddev.y += (sample.y > mean.y) ? (sample.y - mean.y)
                    : (mean.y - sample.y);
        stddev.z += (sample.z > mean.z) ? (sample.z - mean.z)
                    : (mean.z - sample.z);
    }
    stddev.x >>= 2;
    stddev.y >>= 2;
    stddev.z >>= 2;

    unsigned meanmag = mean.x * mean.x + mean.y * mean.y + mean.z * mean.z;
    unsigned stddevmag = stddev.x * stddev.x + stddev.y * stddev.y + stddev.z * stddev.z;
    features.meanmag   = sqrt16(meanmag);
    features.stddevmag = sqrt16(stddevmag);

    switch (__GET(_v_mode)) {
    case MODE_TRAIN_STATIONARY:
    case MODE_TRAIN_MOVING:
        __SET(_v_features.meanmag) = features.meanmag;
        __SET(_v_features.stddevmag) = features.stddevmag;
        //NEXT(4);
        int move_less_error = 0;
        int stat_less_error = 0;
        int i;

        long int meanmag;
        long int stddevmag;
        meanmag = __GET(_v_features.meanmag);
        stddevmag = __GET(_v_features.stddevmag);

        features_t ms, mm;

        for (i = 0; i < MODEL_SIZE; ++i) {
            ms.meanmag = __GET(_v_model_stationary[i].meanmag);
            ms.stddevmag = __GET(_v_model_stationary[i].stddevmag);
            mm.meanmag = __GET(_v_model_moving[i].meanmag);
            mm.stddevmag = __GET(_v_model_moving[i].stddevmag);
            __delay_cycles(100);
            long int stat_mean_err = (ms.meanmag > meanmag)
                                     ? (ms.meanmag - meanmag)
                                     : (meanmag - ms.meanmag);

            long int stat_sd_err = (ms.stddevmag > stddevmag)
                                   ? (ms.stddevmag - stddevmag)
                                   : (stddevmag - ms.stddevmag);

            long int move_mean_err = (mm.meanmag > meanmag)
                                     ? (mm.meanmag - meanmag)
                                     : (meanmag - mm.meanmag);

            long int move_sd_err = (mm.stddevmag > stddevmag)
                                   ? (mm.stddevmag - stddevmag)
                                   : (stddevmag - mm.stddevmag);

            if (move_mean_err < stat_mean_err) {
                move_less_error++;
            } else {
                stat_less_error++;
            }

            if (move_sd_err < stat_sd_err) {
                move_less_error++;
            } else {
                stat_less_error++;
            }
        }

        __SET(_v_class) = (move_less_error > stat_less_error) ? CLASS_MOVING : CLASS_STATIONARY;

        unsigned resultStationaryPct;
        unsigned resultMovingPct;
        unsigned sum;

        ++__SET(_v_totalCount);

        switch (__GET(_v_class)) {
        case CLASS_MOVING:
            ++__SET(_v_movingCount);
            break;
        case CLASS_STATIONARY:
            ++__SET(_v_stationaryCount);
            break;
        }

        if (__GET(_v_totalCount) == SAMPLES_TO_COLLECT) {
            resultStationaryPct = __GET(_v_stationaryCount) * 100 / __GET(_v_totalCount);
            resultMovingPct = __GET(_v_movingCount) * 100 / __GET(_v_totalCount);
            sum = __GET(_v_stationaryCount) + __GET(_v_movingCount);
            NEXT(3);
        } else {
            NEXT(2);
            //NEXT(3);
        }
    case MODE_RECOGNIZE:
        __SET(_v_features.meanmag) = features.meanmag;
        __SET(_v_features.stddevmag) = features.stddevmag;
        //NEXT(5);
        switch (__GET(_v_class)) {
        case CLASS_STATIONARY:
            __SET(_v_model_stationary[__GET(_v_trainingSetSize)].meanmag) = __GET(_v_features.meanmag);
            __SET(_v_model_stationary[__GET(_v_trainingSetSize)].stddevmag) = __GET(_v_features.stddevmag);
            break;
        case CLASS_MOVING:
            __SET(_v_model_moving[__GET(_v_trainingSetSize)].meanmag) = __GET(_v_features.meanmag);
            __SET(_v_model_moving[__GET(_v_trainingSetSize)].stddevmag) = __GET(_v_features.stddevmag);
            break;
        }
        ++__SET(_v_trainingSetSize);
        if (__GET(_v_trainingSetSize) < MODEL_SIZE) {
            //NEXT(3);
            NEXT(2);
        } else {
            NEXT(3);
        }
    default:
        // TODO: abort
        break;
    }
}


TASK(task_idle) { //-->6, NOTE: R() || W()
    if (__GET(_v_count) < 3) {
        __SET(_v_totalCount) = 0;
        NEXT(1);
    } else {
        __SET(_v_totalCount) = 0;
        NEXT(0);
    }
}

/**
 * 0. Benchmark app Init here.
 */
extern uint16_t nvInited;
extern buffer_idx_t elkBufIdx;

void _benchmark_ar_init() {
    if (!nvInited) {
        __THREAD(0);

        
        //        |APP num:1.
        //        |InitSum:0(100us)
        //        |BackupSum:92(100us)
        //        |CksumSum:333(100us)
        //        |UpdateSum:201(100us)
        //        |TaskSum:2365(100us), num:231.
        //        |VerifySum:105(100us)
        //        |Total:3097(100us)
        //         TASK_INIT(0, task_init,             0,      353,     0,       0,     0,     97,     98);  //0
        //         TASK_INIT(0, task_selectMode,       0,      353,     0,      95,     0,     97,     98);  //1
        //         TASK_INIT(0, task_sample,           0,      353,    38,     353,     0,     97,     98);
        //         TASK_INIT(0, task_idle,             0,      353,    96,      97,     0,      5,      2);   //10
                 
                 //        |APP num:1.
                 //        |InitSum:0(100us)
                 //        |BackupSum:129(100us)
                 //        |CksumSum:439(100us)
                 //        |UpdateSum:170(100us)
                 //        |TaskSum:2361(100us), num:231.
                 //        |VerifySum:78(100us)
                 //        |Total:3179(100us)
                 //         TASK_INIT(0, task_init,             0,      353,     0,       0,     0,     353,     354);  //0
                 //         TASK_INIT(0, task_selectMode,       0,      353,     0,      95,     0,     353,     354);  //1
                 //         TASK_INIT(0, task_sample,           0,      353,    38,     353,     0,     353,     354);
                 //         TASK_INIT(0, task_idle,             0,      353,    96,      97,     0,     353,     354);   //10
        
        // |APP num:1.
        // |InitSum:0(100us)
        // |BackupSum:97(100us)
        // |CksumSum:394(100us)
        // |UpdateSum:184(100us)
        // |TaskSum:2356(100us), num:231.
        // |VerifySum:68(100us)
        // |Total:3101(100us)
 //        TASK_INIT(0, task_init,             0,  121);         //0
 //        TASK_INIT(0, task_selectMode,       0, 161);          //1
 //        TASK_INIT(0, task_sample,           38,  353);
 //        TASK_INIT(0, task_idle,             0,  0);          //10
        
        // |APP num:1.
        // |InitSum:0(100us)
        // |BackupSum:92(100us)
        // |CksumSum:345(100us)
        // |UpdateSum:221(100us)
        // |TaskSum:2365(100us), num:231.
        // |VerifySum:119(100us)
        // |Total:3144(100us)
         // TASK_INIT(0, task_init,             0,      97,     0,      0,      0,     97,     98);  //0
         // TASK_INIT(0, task_selectMode,       0,      97,     0,      95,     0,     97,     98);  //1
         // TASK_INIT(0, task_sample,           4,      353,    38,     353,    4,     97,     94);
         // TASK_INIT(0, task_idle,             4,      97,     96,     97,     4,     5,      2);   //10
        
        
//        |APP num:1.
//        |InitSum:0(100us)
//        |BackupSum:105(100us)
//        |CksumSum:394(100us)
//        |UpdateSum:252(100us)
//        |TaskSum:3635(100us), num:263.
//        |VerifySum:135(100us)
//        |Total:4523(100us)
//         TASK_INIT(0, task_init,             0,      97,     0,      0,      0,     97,     98);  //0
//         TASK_INIT(0, task_selectMode,       0,      97,     0,      95,     0,     97,     98);  //1
//         TASK_INIT(0, task_sample,           4,      609,    38,     609,    4,     97,     94);
//         TASK_INIT(0, task_idle,             4,      97,     96,     97,     4,     5,      2);   //10

//        |APP num:1.
//        |InitSum:0(100us)
//        |BackupSum:105(100us)
//        |CksumSum:381(100us)
//        |UpdateSum:229(100us)
//        |TaskSum:3635(100us), num:263.
//        |VerifySum:120(100us)
//        |Total:4472(100us)
//        TASK_INIT(0, task_init,             0,      609,     0,       0,     0,     97,     98);  //0
//        TASK_INIT(0, task_selectMode,       0,      609,     0,      95,     0,     97,     98);  //1
//        TASK_INIT(0, task_sample,           0,      609,    38,     609,     0,     97,     98);
//        TASK_INIT(0, task_idle,             0,      609,    96,      97,     4,     5,       2);   //10

        TASK_INIT(0, task_init,             0,      609,     0,       609,     0,     609,     610);  //0
        TASK_INIT(0, task_selectMode,       0,      609,     0,      609,     0,     609,     610);  //1
        TASK_INIT(0, task_sample,           0,      609,    0,     609,     0,     609,     610);
        TASK_INIT(0, task_idle,             0,      609,    0,      609,     0,     609,       610);   //10


//        |APP num:1.
//        |InitSum:0(100us)
//        |BackupSum:189(100us)
//        |CksumSum:731(100us)
//        |UpdateSum:194(100us)
//        |TaskSum:3630(100us), num:263.
//        |VerifySum:89(100us)
//        |Total:4835(100us)
//         TASK_INIT(0, task_init,             0,      609,     0,       0,     0,     609,     609);  //0
//         TASK_INIT(0, task_selectMode,       0,      609,     0,      95,     0,     609,     609);  //1
//         TASK_INIT(0, task_sample,           0,      609,    38,     609,     0,     609,     609);
//         TASK_INIT(0, task_idle,             0,      609,    96,      97,     0,     609,     609);   //10
//



       // MIN [[0, 81], [0, 353], [4, 345], [0, 353]]
       // |APP num:1.
       // |InitSum:0(100us)
       // |BackupSum:97(100us)
       // |CksumSum:418(100us)
       // |UpdateSum:227(100us)
       // |TaskSum:2363(100us), num:231.
       // |VerifySum:69(100us)
       // |Total:3174(100us)
//        TASK_INIT(0, task_init,             0,  81);         //0
//        TASK_INIT(0, task_selectMode,       0,  353);          //1
//        TASK_INIT(0, task_sample,           4,  345);
//        TASK_INIT(0, task_idle,             0,  353);          //10
//
       // MAX [[0, 351], [0, 353], [0, 353], [132, 135]]
       // |APP num:1.
       // |InitSum:0(100us)
       // |BackupSum:97(100us)
       // |CksumSum:415(100us)
       // |UpdateSum:162(100us)
       // |TaskSum:2363(100us), num:231.
       // |VerifySum:54(100us)
       // |Total:3092(100us)
//        TASK_INIT(0, task_init,             0,      351);         //0
//        TASK_INIT(0, task_selectMode,       0,      353);          //1
//        TASK_INIT(0, task_sample,           0,      353);
//        TASK_INIT(0, task_idle,             132,    135);          //10

        // MAX2 [[0, 351], [0, 353], [0, 353], [2, 217]]
        // |APP num:1.
        // |InitSum:0(100us)
        // |BackupSum:97(100us)
        // |CksumSum:416(100us)
        // |UpdateSum:162(100us)
        // |TaskSum:2370(100us), num:231.
        // |VerifySum:54(100us)
        // |Total:3100(100us)
//        TASK_INIT(0, task_init,             0,      351);         //0
//        TASK_INIT(0, task_selectMode,       0,      353);          //1
//        TASK_INIT(0, task_sample,           0,      353);
//        TASK_INIT(0, task_idle,             2,      217);          //10

        // MIN-interval [[0, 353], [0, 353], [0, 353], [350, 353]]
        // |APP num:1.
        // |InitSum:0(100us)
        // |BackupSum:97(100us)
        // |CksumSum:412(100us)
        // |UpdateSum:180(100us)
        // |TaskSum:2348(100us), num:231.
        // |VerifySum:55(100us)
        // |Total:3093(100us)
//        TASK_INIT(0, task_init,             0,      353);         //0
//        TASK_INIT(0, task_selectMode,       0,      353);          //1
//        TASK_INIT(0, task_sample,           0,      353);
//        TASK_INIT(0, task_idle,             350,    353);          //10
//        |APP num:1.
//        |InitSum:0(100us)
//        |BackupSum:128(100us)
//        |CksumSum:437(100us)
//        |UpdateSum:180(100us)
//        |TaskSum:2348(100us), num:231.
//        |VerifySum:78(100us)
//        |Total:3172(100us)
//        TASK_INIT(0, task_init,             0,      353,     0,      353,      0,      353,     354);  //0
//        TASK_INIT(0, task_selectMode,       0,      353,     0,      353,      0,      349,     350);  //1
//        TASK_INIT(0, task_sample,           0,      353,     0,      353,      0,      353,     354);
//        TASK_INIT(0, task_idle,             350,    353,     350,    353,      0,        0,       0);   //10
//
//        |APP num:1.
//        |InitSum:0(100us)
//        |BackupSum:131(100us)
//        |CksumSum:442(100us)
//        |UpdateSum:160(100us)
//        |TaskSum:2339(100us), num:231.
//        |VerifySum:80(100us)
//        |Total:3154(100us)
//        TASK_INIT(0, task_init,             0,      353,     0,      353,      0,      353,     354);  //0
//        TASK_INIT(0, task_selectMode,       0,      353,     0,      353,      0,      353,     354);  //1
//        TASK_INIT(0, task_sample,           0,      353,     0,      353,      0,      353,     354);
//        TASK_INIT(0, task_idle,             0,      353,     0,      353,      0,      353,     354);   //10


        // MAX-interval [[4, 345], [0, 349], [2, 353], [6, 347]]
        // |APP num:1.
        // |InitSum:0(100us)
        // |BackupSum:97(100us)
        // |CksumSum:427(100us)
        // |UpdateSum:204(100us)
        // |TaskSum:2367(100us), num:231.
        // |VerifySum:69(100us)
        // |Total:3166(100us)
//        TASK_INIT(0, task_init,             4,      345);         //0
//        TASK_INIT(0, task_selectMode,       0,      349);          //1
//        TASK_INIT(0, task_sample,           2,      353);
//        TASK_INIT(0, task_idle,             6,      347);          //10



    } else {
#ifdef ELK
        __THREAD_DUMMY(0, elkBufIdx._idx);
#endif
#ifdef TOTAL
        __THREAD(0);
#endif
    }
}
