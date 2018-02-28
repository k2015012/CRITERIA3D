CDF      
      time       latitude      	longitude            k   HDF_LIB_VERSION       1NCSA HDF Version 4.2 Release 1, February 17, 2005      MACHINE       r720-0-13.local    PROGLANG      F90    date_created      2014-03-09T06:51:57Z   product_version       v05r03     summary       � Included in this file are PATMOS-x/CLAVR-x calibrated reflectance, brightness temperature and cloud products developed by NOAA in collaboration with the University of Wisconsin-Madison CIMSS    cdr_variable      � refl_0_65um_nom, refl_0_86um_nom, refl_1_60um_nom, refl_3_75um_nom, temp_3_75um_nom, temp_11_0um_nom, temp_12_0um_nom, cld_temp_acha, cld_emiss_acha, cld_opd_dcomp, cld_reff_dcomp   institution       < NOAA/NESDIS and the University of Wisconsin-Madison / CIMSS   cdr_program       0 NOAA Climate Data Record Program for satellites   title         J PATMOS-x AVHRR reflectance, brightness temperature and cloud product CDRs     calibration_version       mc5y2011   keywords     3EARTH SCIENCE > LAND SURFACE > SURFACE RADIATIVE PROPERTIES > REFLECTANCE, EARTH SCIENCE > ATMOSPHERE > ATMOSPHERIC RADIATION > REFLECTANCE, EARTH SCIENCE > SPECTRAL/ENGINEERING > INFRARED WAVELENGTHS > BRIGHTNESS TEMPERATURE, EARTH SCIENCE > SPECTRAL/ENGINEERING > SENSOR CHARACTERISTICS > VIEWING GEOMETRY    keywords_vocabulary       NNASA Global Change Master Directory (GCMD) Earth Science Keywords, Version 8.0     time_coverage_resolution      P1D    metadata_link         )gov.noaa.ncdc:C00837 gov.noaa.ncdc.C00840      spatial_resolution        !0.1 x 0.1 degree equal-angle grid      Conventions       CF-1.0     Metadata_Conventions      &CF-1.6, Unidata Dataset Discovery v1.0     standard_name_vocabulary      *CF Standard Name Table (v25, 05 July 2013)     naming_authority      gov.noaa.ncdc      license       %No constraints on data access or use.      sensor        AVHRR/3    platform      NOAA-15    	ACHA_MODE               
DCOMP_MODE              WMO_SATELLITE_CODE           �   #REFL_0_65UM_NOM_DARK_COMPOSITE_NAME       no_file    NAIVE_BAYESIAN_CLOUD_MASK_NAME        (avhrr_default_bayes_mask_no_t11_tstd.txt   	DATA_TYPE         PIXEL      USE_1B_THERMAL_CALIBRATION_FLAG              #USE_1B_REFLECTANCE_CALIBRATION_FLAG              RENAVIGATION_FLAG                USE_SST_ANALYSIS_FLAG               SST_ANALYSIS_SOURCE_FLAG             NWP_FLAG             MODIS_CLEAR_SKY_REFLECTANCE_FLAG            CH1_GAIN_LOW      =y�\   CH1_GAIN_HIGH         >;]�   CH1_SWITCH_COUNT      C�     CH1_DARK_COUNT        B�   CH2_GAIN_LOW      =�e'   CH2_GAIN_HIGH         >O��   CH2_SWITCH_COUNT      C�     CH2_DARK_COUNT        B��   CH3A_GAIN_LOW         <�G�   CH3A_GAIN_HIGH        >=�   CH3A_SWITCH_COUNT         C�     CH3A_DARK_COUNT       Dw:v   SUN_EARTH_DISTANCE        ?~{�   C1        7Gӹ   C2        ?�*&   A_20      ����   B_20      ?�A-   NU_20         E(�   A_31      ��.0   B_31      ?�*   NU_31         DgZ   A_32      ��   B_32      ?�    NU_32         DQ�w   SOLAR_20_NU       Añ   TIME_ERROR_SECONDS               CLOUD_MASK_VERSION        K$Id: naive_bayesian_cloud_mask_module.f90 9 2014-01-31 08:19:35Z awalther $    CLOUD_MASK_THRESHOLDS_VERSION         W$Id: avhrr_default_bayes_mask_no_t11_tstd.txt,v 1.1 2014/01/27 03:26:05 heidinger Exp $    CLOUD_TYPE_VERSION        A$Id: universal_cloud_type.f90 37 2014-02-05 22:22:48Z heidinger $      ACHA_VERSION      =$Id: awg_cloud_height.f90 68 2014-02-14 22:13:26Z heidinger $      DCOMP_VERSION         7$Id: dcomp_array.f90 85 2014-02-25 17:59:58Z awalther $    NUMBER_OF_ELEMENTS          �   PROCESSING_TIME_MINUTES       @R7    NONCONFIDENT_CLOUD_MASK_FRACTION      >w�   ACHA_SUCCESS_FRACTION         ?�?   DCOMP_SUCCESS_FRACTION        ?xN   geospatial_lon_number              geospatial_lat_number              geospatial_lon_spacing        =���   geospatial_lat_spacing        =���   geospatial_lat_units      degrees_north      geospatial_lon_units      degrees_east   geospatial_lon_min        @#F@      geospatial_lon_max        @)�       geospatial_lat_max        @F�'       geospatial_lat_min        @F.       cdm_data_type         Grid   time_coverage_start       1998-10-26T00:00:00Z   time_coverage_end         1998-10-26T23:59:59Z   id        1patmosx_v05r03_noaa-15_des_d19981026_c20140309.nc      	DATA_NODE         des    COMPOSITE_TIME_HOURS      0000   COMPOSITE_TIME_WINDOW_HOURS       00     SENSOR_ZENITH_SAMPLING_METHOD         
MOST_NADIR     SPATIAL_SAMPLING_METHOD       NEAREST_NEIGHBOR   source       iNSS.GHRR.NK.D98299.S0054.E0248.B0235153.GC; NSS.GHRR.NK.D98299.S0243.E0429.B0235354.GC; NSS.GHRR.NK.D98299.S0424.E0611.B0235455.GC; NSS.GHRR.NK.D98299.S0606.E0800.B0235556.WI; NSS.GHRR.NK.D98299.S0755.E0949.B0235657.WI; NSS.GHRR.NK.D98299.S0944.E1137.B0235758.WI; NSS.GHRR.NK.D98299.S1133.E1316.B0235859.WI; NSS.GHRR.NK.D98299.S1311.E1505.B0235960.GC; NSS.GHRR.NK.D98299.S1502.E1632.B0236061.GC; NSS.GHRR.NK.D98299.S1627.E1813.B0236162.GC; NSS.GHRR.NK.D98299.S1808.E1953.B0236263.GC; NSS.GHRR.NK.D98299.S1948.E2132.B0236364.GC; NSS.GHRR.NK.D98299.S2128.E2257.B0236465.WI; NSS.GHRR.NK.D98299.S2252.E0036.B0236466.WI;        FRACTION_WITH_DATA        ?~�j   FRACTION_WITH_VALID_DATA      ?~�j   FRACTION_WITH_DAYTIME_DATA        ?hځ   FRACTION_WITH_CLOUDY_DATA         ?6�)   FRACTION_WITH_ICE_CLOUDY_DATA         >�   MEAN_DAY_REFLECTANCE_0_65UM_NOM       B ��   MEAN_DAY_REFLECTANCE_0_86UM_NOM       B��   MEAN_DAY_REFLECTANCE_1_60UM_NOM       �y�    MEAN_TEMPERATURE_3_750UM_NOM      C��   MEAN_TEMPERATURE_11_0UM_NOM       C�4   MEAN_TEMPERATURE_12_0UM_NOM       C��m   _History      ,Direct read of HDF4 file through CDM library   HDF4_Version      94.2.1 (NCSA HDF Version 4.2 Release 1, February 17, 2005)      history       12014-03-09T06:51:57Z file produced using mc5y2011      History      Translated to CF-1.0 Conventions by Netcdf-Java CDM (NetcdfCFWriter)
Original Dataset = /data/Personal/mhiley/best_patmosx/Satellite_Output/avhrr/global/1998/level2b_daily/NetCDF/patmosx_v05r03_NOAA-15_des_d19981026_c20140309.nc; Translation Date = Wed Jul 27 12:51:32 UTC 2016            cloud_fraction                        standard_name         cloud_area_fraction    	long_name         ^cloud fraction computed over a 3x3 pixel array at the native resolution centered on this pixel     coordinates       time latitude longitude    actual_range          ?�     scaled                T  2�   time                axis      T      units         !seconds since 1970-01-01 00:00:00      	long_name         !seconds since 1970-01-01 00:00:00      standard_name         time   _CoordinateAxisType       Time        8   latitude               units         degrees_north      standard_name         latitude   	long_name         latitude   actual_range      ´  B�     axis      Y      scaled              _CoordinateAxisType       Lat       ,  2   	longitude                  units         degrees_east   standard_name         	longitude      	long_name         	longitude      actual_range      �4  C4     axis      X      scaled              _CoordinateAxisType       Lon       |  24   
dcomp_info                        	long_name        %processing flags for DCOMP 1:info flag set ? (0=no,1=yes) 2:land/sea mask (0=land,1=sea) 3:day/night mask (0=Day,1=Night) 4:twilight (65-82 solar zenith) (0=no,1=yes) 5:snow (0=no,1= snow) 6:sea-ice (0=no,1=sea-ice) 7:phase (0=liquid,1=ice) 8:thick_cloud (0=no,1=yes) 9:thin_cloud (0=no,1=yes)      coordinates       time latitude longitude    
flag_masks                @ �   flag_meanings         wland_or_sea_mask day_or_night_mask twilight_@65-82_solar_zenith snow sea_ice liquid_or_ice_phase thick_cloud thin_cloud    scaled                 �  8   scan_line_time                        units         hours      	long_name         *time for the scan line in fractional hours     coordinates       time latitude longitude    
_FillValue        �y�    scaled                 T  :�   dcomp_quality                         	long_name        quality flags for DCOMP products 1:Processed (0=no,1=yes) 2:valid COD retrieval (0=yes,1=no) 3:valid REF retrieval (0=yes,1=no) 4:degraded COD retrieval (0=no,1=degraded) 5:degraded REF retrieval (0=no,1=degraded) 6:convergency (0=no,1=yes) 7:glint (0=no,1=yes)      coordinates       time latitude longitude    
flag_masks         @    flag_meanings         qProcessed valid_COD_retrieval valid_REF_retrieval degraded_COD_retrieval degraded_REF_retrieval convergency glint      scaled                 X  @   
snow_class                        	long_name         6snow classes and values:no snow/ice=1,sea_ice=2,snow=3     coordinates       time latitude longitude    
_FillValue        �      flag_values           flag_meanings         no_snow_or_ice  sea_ice  snow      scaled                 X  A`   
cloud_mask                        	long_name         iinteger classification of the cloud mask including clear=0, probably-clear=1, probably-cloudy=2, cloudy=3      coordinates       time latitude longitude    
_FillValue        �      flag_values           flag_meanings         /clear  probably_clear  probably_cloudy  cloudy     scaled                 X  B�   solar_zenith_angle                        units         degrees    standard_name         solar_zenith_angle     	long_name         Tsolar zenith for each pixel measured in degrees away from the sun (0=looking at sun)   coordinates       time latitude longitude    actual_range          C4     scaled                T  D   
land_class                        	long_name         �land classes and values:shallow ocean=0,land=1,coastline=2,shallow inland water=3,ephemeral water=4,deep inland water=5,moderate ocean=6,deep ocean=7      coordinates       time latitude longitude    
_FillValue        �      flag_values           flag_meanings         mocean  land  coastline  shallow_inland_water  ephemeral_water  deep_inland_water  moderate_ocean  deep_ocean       scaled                 X  Id   surface_type                      standard_name         	area_type      	long_name         �UMD surface type: water=0,evergreen_needle=1,evergreen_broad=2,deciduous_needle=3,deciduous_broad=4,mixed_forest=5,woodlands=6,wooded_grass=7closed_shrubs=8,open_shrubs=9,grasses=10,croplands=11,bare=12,urban=13    coordinates       time latitude longitude    
_FillValue        �      flag_values        	
     flag_meanings         �water evergreen_needle evergreen_broad deciduous_needle deciduous_broad mixed_forest woodlands wooded_grass closed_shrubs open_shrubs grasses croplands bare urban     scaled                 X  J�   sensor_zenith_angle                       units         degrees    standard_name         sensor_zenith_angle    	long_name         ;sensor zenith for each pixel measured in degrees from nadir    coordinates       time latitude longitude    actual_range          B�     scaled                T  L   	acha_info                         	long_name         �processing information for ACHA (0=no/1=yes) 1:Cloud Height Attempted 2:Bias Correction Employed 3:Ice Cloud Retrieval 4:Local Radiative Center Processing Used 5:Multi-layer Retrieval 6:Lower Cloud Interpolation Used 7:Boundary Layer Inversion Assumed    coordinates       time latitude longitude    
flag_masks         @    flag_meanings         �Cloud_Height_Attempted Bias_Correction_Employed Ice_Cloud_Retrieval Local_Radiative_Center_Processing_Used Multi_Layer_Retrieval Lower_Cloud_Interpolation_Used Boundary_Layer_Inversion_Assumed   scaled                 X  Qh   acha_quality                      	long_name        quality flags for ACHA products 1:Processed (0=no,1=yes) 2:valid Tc retrieval (1=yes,0=no) 3:valid ec retrieval (1=yes,0=no) 4:valid beta retrieval (1=yes,0=no) 5:degraded Tc retrieval (1=yes,0=no) 6:degraded ec retrieval (1=yes,0=no) 7:degraded beta retrieval (1=yes,0=no)      coordinates       time latitude longitude    
flag_masks         @    flag_meanings         �Processed valid_Tc_retrieval valid_ec_retrieval valid_beta_retrieval degraded_Tc_retrieval degraded_ec_retrieval degraded_beta_retrieval   scaled                 X  R�B01pB0��B0��B1dB1�B21TB2�eB2��B3c�B3��B418A�2AW5A�8A��A,�A ��A"V�A#��A%�KA',NA(�RA*VUA+�XA-��A/+�A0��A2U�A3��A5�kA7+nA8�qA:UuA;�xA=��A?*�A@�ABUAC�AE��AG*�AH��>e˗>�V�?*T�?*T�?*T�>�Ǐ>e˗=�Ө1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   =�Ө>�Ǐ?*T�?bŋ>�Ǐ>�V�=�Ө=�Ө>e˗>�V�>�V�=�Ө=�Ө=�Ө>�V�>�V�>e˗1   1   1   1   1   1   1   1   1   1   =�Ө>�V�?*T�?~��?~��?bŋ?F�?F�>�V�1   1   >e˗1   1   1   =�Ө=�Ө=�Ө=�Ө1   1   1   1   1   1   1   1   1   1   >e˗?8?F�?bŋ?bŋ?F�?F�?F�?bŋ?bŋ1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   >e˗>�V�?*T�?~��?~��?T�S?T�S?~��?~��?~��?~��?~��=�Ө=�Ө=�Ө>-Z�1   1   1   1   1   1   1   1   1   1   1   1   1   =�Ө>e˗?8?F�?bŋ?*T�?8?*T�?*T�?bŋ?~��?~��?~��?~��1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   1   >�Ǐ?*T�?bŋ?~��?F�?*T�?8?F�?bŋ?~��?~��?~��?~��?bŋ1   1   1   1   1   1   1   1   1   1   1   =�Ө>�V�?*T�?F�?*T�?bŋ?bŋ?bŋ?*T�?F�?8?*T�?8?F�?bŋ?F�?~��?~��?~��?~��1   1   1   1   1   1   1   1   =�Ө=�Ө>�Ǐ?*T�?F�?*T�?bŋ?*T�>�V�1   1   1   1   1   1   1   >e˗?8?F�?~��?~��?~��?~��1   1   1   1   1   1   =�Ө>�V�>e˗?*T�?~��?~��?F�>�Ǐ>�Ǐ1   >e˗>e˗>e˗1   1   =�Ө>e˗>�V�>e˗>�V�?8?bŋ?~��?~��?~��1   1   1   1   1   1   >�V�?*T�?F�?~��?F�>�Ǐ>e˗=�Ө=�Ө=�Ө1   1   1   >�V�=�Ө>�V�>�Ǐ?8?8?8?F�?bŋ?~��?bŋ?*T�1   1   1   1   1   >e˗?8?F�?bŋ?F�>�Ǐ>e˗>�V�>e˗>e˗=�Ө>�V�>�Ǐ>�V�?F�?*T�?F�?F�>�Ǐ=�Ө>�V�?8?F�?F�?*T�>�V�NX�� "  "   # # # % # % % % % % % % % % % % % % % % % % % %  % " " # # #  # # # #   % % % % % % % % % % % % %       # # # # # # # # # # % % % % % % % % % % % % %     % %   # # # # # # # # # # # % % % % % % % % % %    %       #  # # # # # # # # # % # % % % % % %   %   %       # # # # # # # # # # # # # % % % % %      %        # # # # # # # # # # # # #        %           # # # # # # # # # # #      % % % % % % % % % %      # # # # # # # # #      # % %  % % % % % % % % %     # # # # # # #     # # #  # % % % % % % %         # # # # # # #     # #  # # %   %    % % %  $   $�@Ի@Ը�@Ը�@Ը�@Ի@Է�@Է�@Է�@Է�@Զ@Զ@Զ@Զ@Զ@Զ@Ե[@Ե[@Ե[@Ե[@Ե[@Ե[@Դ7@Դ7@Դ7@Դ7@Գ@Գ@Գ@Գ@Գ@Ա�@Դ7@Ե[@Ե[@Ե[@Դ7@Դ7@Դ7@Դ7@Ա�@Գ@Գ@Գ@Գ@Գ@Ա�@Ա�@Ա�@Ա�@Ա�@԰�@԰�@԰�@԰�@԰�@԰�@ԯ�@Ԯ�@Ԯ�@Ԯ�@Ԯ�@Ԯ�@Ա�@԰�@԰�@Գ@԰�@԰�@԰�@Ա�@ԯ�@ԯ�@ԯ�@ԯ�@ԯ�@ԭc@Ԯ�@Ԯ�@Ԯ�@ԭc@ԭc@ԭc@ԭc@ԬA@ԬA@ԬA@ԬA@ԫ@ԫ@ԫ@ԫ@ԫ@ԩ�@ԭc@ԭc@Ԯ�@ԬA@ԬA@ԬA@ԬA@ԫ@ԬA@ԬA@ԫ@ԫ@ԩ�@ԫ@ԫ@ԩ�@ԩ�@ԩ�@ԩ�@ԫ@Ԩ�@Ԩ�@Ԩ�@Ԩ�@ԧ�@ԧ�@ԧ�@ԧ�@ԧ�@Ԧ�@Ԧ�@ԩ�@ԩ�@ԫ@Ԩ�@Ԩ�@Ԩ�@ԫ@ԧ�@ԧ�@ԧ�@ԧ�@ԩ�@Ԧ�@Ԧ�@Ԧ�@Ԧ�@Ԧ�@Ԧ�@ԥm@ԥm@ԥm@ԥm@ԤI@ԤI@ԤI@ԤI@ԤI@ԣ&@ԣ&@ԣ&@ԣ&@ԥm@Ԧ�@ԥm@ԥm@ԥm@ԥm@ԤI@ԤI@ԤI@ԤI@Ԧ�@ԣ&@ԣ&@ԣ&@ԣ&@ԣ&@ԤI@Ԣ@Ԣ@Ԣ@Ԣ@Ԡ�@Ԡ�@Ԡ�@Ԡ�@Ԡ�@ԟ�@ԟ�@ԟ�@ԟ�@ԟ�@Ԣ@ԤI@Ԡ�@Ԡ�@Ԡ�@ԣ&@Ԡ�@Ԡ�@Ԡ�@Ԡ�@ԟ�@ԟ�@ԟ�@ԟ�@ԟ�@Ԡ�@Ԟ�@Ԟ�@Ԟ�@ԝv@ԝv@ԝv@ԝv@ԝv@ԝv@ԝv@ԜS@ԜS@ԛ0@ԛ0@ԛ0@Ԟ�@ԟ�@ԝv@ԝv@Ԟ�@ԝv@ԜS@ԜS@ԜS@Ԟ�@ԜS@ԜS@ԜS@ԛ0@ԝv@ԛ0@ԛ0@Ԛ@Ԛ@Ԛ@Ԛ@Ԛ@Ԛ@Ԙ�@Ԙ�@Ԙ�@ԗ�@ԗ�@ԗ�@ԗ�@ԗ�@ԜS@Ԛ@Ԛ@Ԛ@ԜS@Ԙ�@Ԙ�@Ԙ�@ԛ0@Ԙ�@ԗ�@ԗ�@ԗ�@Ԙ�@ԗ�@Ԗ�@Ԗ�@Ԗ�@Ԗ�@Ԗ�@Ԗ�@ԕ@ԕ@ԕ@ԕ@Ԕ\@Ԕ\@Ԕ\@Ԕ\@Ԕ\@ԓ9@Ԙ�@ԕ@ԕ@Ԗ�@Ԕ\@ԕ@ԕ@ԕ@Ԕ\@Ԕ\@Ԕ\@Ԕ\@Ԕ\@ԓ9@ԓ9@ԓ9@ԓ9@ԓ9@ԓ9@Ԑ�@Ԓ@Ԓ@Ԓ@Ԑ�@Ԑ�@Ԑ�@Ԑ�@Ԑ�@ԏ�@ԏ�@ԏ�@Ԑ�@Ԓ@Ԓ@Ԕ\@Ԓ@Ԓ@Ԓ@ԓ9@Ԑ�@Ԑ�@Ԑ�@Ԑ�@ԏ�@ԏ�@ԏ�@ԏ�@ԏ�@ԏ�@ԍ�@Ԏ�@Ԏ�@Ԏ�@ԍ�@ԍ�@ԍ�@ԍ�@ԍ�@Ԍe@Ԍe@Ԍe@Ԍe>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>������                                                                                                                                                                             ���B�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B���B���B���B���B���B���B���B���B���B���B���B��&B��&B��&B�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B���B���B���B���B���B���B���B���B���B���B���B��&B��&B�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B���B���B���B���B���B���B���B���B���B���B���B���B��&B�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B���B���B���B���B���B���B���B���B���B���B���B��&B�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B���B���B���B���B���B���B���B���B���B���B���B�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B���B���B���B���B���B���B���B���B���B���B�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B���B���B���B���B���B���B���B���B���B���B�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B���B���B���B���B���B���B���B���B���B�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B���B���B���B���B���B���B���B���B�ӧB�>}B�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B���B���B���B���B���B���B���B�>}B�>}B�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�ӧB�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B�h�B���B���B���B���B���B���B���                            ���     
  
 

  



   



    



     


      

   

  
 
 
 

   
 
   ���B]4B]4B[�_BZD�BXٳBXٳBWn�BVBT�2BT�2BS.]BQÇBPX�BN��BM�BM�BL0BJ�[BIB�BGׯBFl�BEBC�.BB,YB@��B?V�B=��B<�B;,B9�WB8@�B^�
B]4B[�_BZD�BZD�BXٳBWn�BVBVBT�2BS.]BQÇBPX�BN��BN��BM�BL0BJ�[BIB�BGׯBFl�BEBC�.BB,YB@��B@��B?V�B=��B<�B;,B9�WB^�
B]4B[�_BZD�BZD�BXٳBWn�BVBVBT�2BS.]BQÇBPX�BPX�BN��BM�BL0BJ�[BIB�BGׯBFl�BFl�BEBC�.BB,YB@��B?V�B=��B<�B;,B9�WB^�
B]4B[�_B[�_BZD�BXٳBWn�BWn�BVBT�2BS.]BQÇBQÇBPX�BN��BM�BL0BJ�[BIB�BGׯBGׯBFl�BEBC�.BB,YB@��B?V�B=��B<�B;,B9�WB^�
B]4B[�_B[�_BZD�BXٳBWn�BWn�BVBT�2BS.]BQÇBQÇBPX�BN��BM�BL0BJ�[BJ�[BIB�BGׯBFl�BEBC�.BB,YB@��B?V�B=��B<�B;,B9�WB^�
B]4B]4B[�_BZD�BXٳBXٳBWn�BVBT�2BQÇBS.]BQÇBPX�BN��BM�BL0BL0BJ�[BIB�BGׯBFl�BEBC�.BB,YB@��B?V�B=��B<�B;,B9�WB^�
B]4B]4B[�_BZD�BXٳBXٳBWn�BVBT�2BT�2BS.]BQÇBPX�BN��BL0BM�BL0BJ�[BIB�BGׯBFl�BEBC�.BB,YB@��B@��B?V�B=��B<�B;,B^�
B]4B]4B[�_BZD�BZD�BXٳBWn�BVBT�2BT�2BS.]BQÇBPX�BM�BN��BM�BL0BJ�[BIB�BGׯBFl�BEBEBC�.BB,YB@��B?V�B=��B<�B;,B]4B^�
B]4B[�_BZD�BZD�BXٳBWn�BVBVBT�2BS.]BQÇBPX�BPX�BN��BM�BL0BJ�[BIB�BGׯBGׯBFl�BEBC�.BB,YB@��B?V�B=��B<�B;,B^�
B^�
B]4B[�_B[�_BZD�BXٳBWn�BWn�BVBT�2BS.]BQÇBQÇBPX�BN��BM�BL0BJ�[BJ�[BIB�BGׯBFl�BEBC�.BB,YB@��B?V�B=��B<�B;,B_��B^�
B]4B[�_B[�_BZD�BXٳBWn�BWn�BVBT�2BS.]BS.]BQÇBPX�BN��BM�BL0BL0BJ�[BIB�BGׯBFl�BEBC�.BB,YB@��B?V�B=��B<�B;, I II                        	      	    		                		                           	   I                          		III 	                 		 	   	IIII                  			     IIIII                  		 	   IIIIII               	          		III         	 	     	         		II       		I             I	 				I       I 	       		 			   	 I  ��� o oo                        o      o    Oo                oo                           o   o                          ooooo o                 oo o   ooooo                  ooo     ooooo                  oo o   oooooo               o          ooooo         o o     o         oooo       ooo             oo ooooo       o o       oo ooo   o o  ���