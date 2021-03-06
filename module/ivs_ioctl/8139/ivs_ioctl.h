#ifndef _IOCTL_IVS_H_

    #define _IOCTL_IVS_H_
     
 	#define IVS_VER           	0x00200100
 	//                            HIIFFFBB
    //                            H 			huge change
    //                             II 			interface change
    //                               FFF 		functional modified or bug fixed
    //                                  BB 	    branch for customer request

	#define IVS_INT_VER       	0x13031100 	//	added for internal release only
    //                            YYMMDDSS 
    //                            YY 			year
    //                              MM 			month
    //                                DD 		date
    //                                  SS 	    sequence number

    #define IVS_VER_MAJOR     	((IVS_VER>>28)&0x0f)   // huge change
    #define IVS_VER_MINOR     	((IVS_VER>>20)&0xff)   // inerface change
    #define IVS_VER_MINOR2    	((IVS_VER>>8)&0xfff)   // functional modified or bug fixed
    #define IVS_VER_BRANCH    	((IVS_VER>>0)&0x0ff)   // branch for customer request
	
    #define IVS_INIT                    0x4170
    #define IVS_INIT_MEM                0x4171
    #define IVS_HSI_CONVERSION          0x4172
    #define IVS_RGB_CONVERSION          0x4173    
    #define IVS_INTEGRAL_IMAGE          0x4174
    #define IVS_SQUARED_INTEGRAL_IMAGE  0x4175
    #define IVS_DE_INTERLEAVING         0x4176
    #define IVS_HISTOGRAM               0x4177
    #define IVS_CONVOLUTION             0x4178
    #define IVS_MORPHOLOGY              0x4179
    #define IVS_SAD                     0x4180
    #define IVS_RASTER_OPERATION        0x4181 
    #define IVS_CASCADED_CLASSIFIER     0x4182

    #define IVS_DEV  				    "/dev/ivs"
    #define MAX_IVS_NUM    			    32

	typedef struct ivs_ioctl_io_t {
		int addr;
		int size;
		int offset;
	} ivs_ioctl_io_t;

	typedef struct ivs_ioctl_mem_t {
        int memory_size;
        int memory_num;
	} ivs_ioctl_mem_t;

	typedef struct ivs_io_t {
		int size;
        int faddr;
        int offset;
        unsigned char *vaddr;
	} ivs_io_t;

	typedef struct ivs_img_t {
		int img_format;
        int img_height;
        int img_width;
        int swap_y_cbcr;
        int swap_endian;
	} ivs_img_t;

	typedef struct ivs_operation_t{
		int operation ;
		int kernel_template_index ;
		int shifted_convolution_sum ;
		int morphology_operator ;
		int select_threshold_output ; 
		int threshold ;
		int block_size ;
        int convolution_tile;
		int raster_operationo_code ;
		int integral_cascade ; 
		int template_element[25] ;
	} ivs_operation_t;

	typedef struct ivs_ioctl_param_t {
		ivs_ioctl_io_t  input_img_1;
		ivs_ioctl_io_t  input_img_2;

		ivs_ioctl_io_t  output_rlt_1;
		ivs_ioctl_io_t  output_rlt_2;
		ivs_ioctl_io_t  output_rlt_3;

		ivs_img_t       img_info;
		ivs_ioctl_mem_t mem_info;
		ivs_operation_t	operation_info;
	} ivs_ioctl_param_t ;

	typedef struct ivs_param_t {
		ivs_io_t  input_img_1;
		ivs_io_t  input_img_2;

		ivs_io_t  output_rlt_1;
		ivs_io_t  output_rlt_2;
		ivs_io_t  output_rlt_3;
	
		ivs_img_t       img_info;
		ivs_operation_t	operation_info;
	} ivs_param_t ;	
      
	typedef struct ivs_info_t{
		int fd;
		unsigned char *mmap_addr;
	} ivs_info_t;

    int ivs_hsi_conversion(ivs_param_t *ivs_param);
    int ivs_rgb_conversion(ivs_param_t *ivs_param);
    int ivs_integral_image(ivs_param_t *ivs_param);
    int ivs_squared_integral_image(ivs_param_t *ivs_param);
    int ivs_de_interleaving(ivs_param_t *ivs_param);
    int ivs_histogram(ivs_param_t *ivs_param);
    int ivs_convolution(ivs_param_t *ivs_param);
    int ivs_morphology(ivs_param_t *ivs_param);
    int ivs_sad(ivs_param_t *ivs_param);
    int ivs_raster(ivs_param_t *ivs_param);
    int ivs_cascaded_classifier(ivs_param_t *ivs_param);
    
#endif
