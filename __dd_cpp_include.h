#ifndef DD_INIT_LAYER_COUNT
  #define DD_INIT_LAYER_COUNT  5
#endif
#ifndef DD_LAYER_COUNT_INC
  #define DD_LAYER_COUNT_INC   3
#endif  
#if DD_INIT_LAYER_COUNT < 1
  #error "DD_INIT_LAYER_COUNT must be at least 1"
#endif 
#if DD_LAYER_COUNT_INC < 2
  #error "DD_LAYER_COUNT_INC must be at least 2"
#endif 



#ifdef DD_NO_PASSIVE_CONNECT
  #warning ??? DD_NO_PASSIVE_CONNECT set ???
#else
  #define SUPPORT_PASSIVE
  #define SUPPORT_MASTER_RESET 
#endif

#ifdef DD_NO_FEEDBACK
  #warning ??? DD_NO_FEEDBACK set ???
#else
  #define ENABLE_FEEDBACK
#endif

#ifdef ENABLE_FEEDBACK
  //#define FEEDBACK_BUFFER_SIZE 4
  #define HANDLE_FEEDBACK_DURING_DELAY
  #define READ_BUFFER_USE_BUFFER
  #ifndef DD_NO_FEEDBACK_BYTES
    #define FEEDBACK_SUPPORT_BYTES
  #endif   
#endif
