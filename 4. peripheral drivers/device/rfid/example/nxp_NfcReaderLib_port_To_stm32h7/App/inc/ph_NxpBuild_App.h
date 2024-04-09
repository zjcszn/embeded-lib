/*----------------------------------------------------------------------------*/
/* Copyright 2016-2023 NXP                                                    */
/*                                                                            */
/* NXP Confidential. This software is owned or controlled by NXP and may only */
/* be used strictly in accordance with the applicable license terms.          */
/* By expressly accepting such terms or by downloading, installing,           */
/* activating and/or otherwise using the software, you are agreeing that you  */
/* have read, and that you agree to comply with and are bound by, such        */
/* license terms. If you do not agree to be bound by the applicable license   */
/* terms, then you may not retain, install, activate or otherwise use the     */
/* software.                                                                  */
/*----------------------------------------------------------------------------*/

/** \file
* Application specific selection of Reader Library Components
*
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*
*/

#ifndef PH_NXPBUILD_APP_H_INC
#define PH_NXPBUILD_APP_H_INC

/** \defgroup ph_NxpBuild NXP Build
* \brief Controls the Inclusion of required components, Inclusion SRC/DATA within components and the Build Dependencies between the components
* @{
*/

/* NXPBUILD_DELETE: included code lines should be always removed from code */

/* NXP BUILD DEFINES */
/* use #define to include components            */
/* comment out #define to exclude components    */


/* DEBUG build mode */
/*#define NXPBUILD__PH_DEBUG*/                              /**< DEBUG build definition */


//#define NXPRDLIB_REM_GEN_INTFS
/*********************************************************************************************************************************************************************************/



/*********************************************************************************************************************************************************************************/

#define NXPBUILD__PHPAL_I14443P3A_SW                        /**< PAL ISO 14443-3A SW Component is included. */

#define NXPBUILD__PHPAL_I14443P3B_SW                        /**< PAL ISO 14443-3B SW Component is included. */

#define NXPBUILD__PHPAL_I14443P4A_SW                        /**< PAL ISO 14443-4A SW Component is included. */

#define NXPBUILD__PHPAL_I14443P4_SW                         /**< PAL ISO 14443-4 SW Component is included. */

#define NXPBUILD__PHPAL_MIFARE_SW                           /**< PAL MIFARE SW Component is included */

//#define NXPBUILD__PHPAL_FELICA_SW                           /**< PAL Felica SW Component is included. */

#define NXPBUILD__PHPAL_SLI15693_SW                         /**< PAL SLI 15693 Component is included. */

//#define NXPBUILD__PHPAL_I18000P3M3_SW                       /**< PAL ISO 18000p3m3 Component is included. */

//#define NXPBUILD__PHPAL_EPCUID_SW                           /**< PAL EPC UID SW Component is included. */

//#define NXPBUILD__PHPAL_I18092MPI_SW                        /**< PAL ISO18092 (P2P) SW Component is included. */

/*********************************************************************************************************************************************************************************/

#define NXPBUILD__PHAC_DISCLOOP_SW                          /**< Discovery Loop Activity SW Component is included. */

#ifdef NXPBUILD__PHAC_DISCLOOP_SW                           /**< If DiscLoop SW Component is included,  macros( & it's dependencies) to include/exclude SRC/DATA within Discloop is defined. */

#if defined (NXPBUILD__PHHAL_HW_RC663)
//#define NXPBUILD__PHAC_DISCLOOP_LPCD       /**< SRC to enable LPCD is included. */
#endif

#ifdef  NXPBUILD__PHPAL_I14443P3A_SW
#define NXPBUILD__PHAC_DISCLOOP_TYPEA_I3P3_TAGS     /**< SRC/DATA to Detect/CollRes/Activate cards such as MFC, MFUL, MFP SL1 etc is included. */
#define NXPBUILD__PHAC_DISCLOOP_TYPEA_JEWEL_TAGS    /**< SRC/DATA to Detect cards such as NFC Forum T1T, Topaz/Jewel is included. */

#if defined(NXPBUILD__PHPAL_I14443P4A_SW) && defined(NXPBUILD__PHPAL_I14443P4_SW)
#define NXPBUILD__PHAC_DISCLOOP_TYPEA_I3P4_TAGS  /**< SRC/DATA to Detect cards such as MFDF, MFP, T4AT NFC Forum Tag or Type A EMVCo is included. */
#endif
#endif /* NXPBUILD__PHPAL_I14443P3A_SW */

#if defined(NXPBUILD__PHPAL_I14443P3A_SW) && defined(NXPBUILD__PHPAL_I18092MPI_SW)
#define NXPBUILD__PHAC_DISCLOOP_TYPEA_P2P_TAGS      /**< SRC/DATA to Detect Peer Passive Type A P2P Target mode devices is included. */
#endif

#ifdef NXPBUILD__PHPAL_FELICA_SW
#define NXPBUILD__PHAC_DISCLOOP_FELICA_TAGS                /**< SRC/DATA to Detect FeliCa Cards is included. */
#ifdef  NXPBUILD__PHPAL_I18092MPI_SW
#define NXPBUILD__PHAC_DISCLOOP_TYPEF_P2P_TAGS         /**< SRC/DATA to Detect Peer Passive Type F P2P Target mode devices is included. */
#endif /* NXPBUILD__PHPAL_I18092MPI_SW */
#endif /* NXPBUILD__PHPAL_FELICA_SW */

#ifdef NXPBUILD__PHPAL_I14443P3B_SW
#define NXPBUILD__PHAC_DISCLOOP_TYPEB_I3P3B_TAGS           /**< SRC/DATA to Detect Type B Cards that operate at Layer3 level is included */
#ifdef NXPBUILD__PHPAL_I14443P4_SW
#define NXPBUILD__PHAC_DISCLOOP_TYPEB_I3P4B_TAGS       /**< SRC/DATA to Detect Type B Cards such as NFC Forum Type 4 Tags, EMVCo Type B Cards etc is included */
#endif /* NXPBUILD__PHPAL_I14443P4_SW */
#endif /* NXPBUILD__PHPAL_I14443P3B_SW */

#ifdef NXPBUILD__PHPAL_SLI15693_SW
#define NXPBUILD__PHAC_DISCLOOP_TYPEV_TAGS                 /**< SRC/DATA to Detect Type V Cards such as ICODE SLI/SLIX/SLI2/Tesa Cards is included*/
#endif /* NXPBUILD__PHPAL_SLI15693_SW */

#ifdef NXPBUILD__PHPAL_I18000P3M3_SW
#define NXPBUILD__PHAC_DISCLOOP_I18000P3M3_TAGS            /**< SRC/DATA to Detect ICODE ILT Cards such as  SMARTRAC StackIt Cards is included*/
#endif /* NXPBUILD__PHPAL_I18000P3M3_SW */

#endif /* NXPBUILD__PHAC_DISCLOOP_SW */

/*********************************************************************************************************************************************************************************/

#define NXPBUILD__PHNFCLIB                                  /**< Simplified API Interface, If enabling this the entry point should be this component in the application */

/*********************************************************************************************************************************************************************************/

//#define NXPBUILD__PH_CIDMANAGER_SW                            /**< CID Manager SW Component is included. */

#define NXPBUILD__PH_KEYSTORE_SW                                /**< SW KeyStore Component is included. */

#if defined(NXPBUILD__PHHAL_HW_RC663) && !defined(NXPBUILD__PH_KEYSTORE_SW)
#define NXPBUILD__PH_KEYSTORE_RC663                         /**< RC663 KeyStore Component is included. */
#endif

//#define NXPBUILD__PH_NDA_MFDF                                 /**< MIFARE DESFire contactless IC */

#define NXPBUILD__PH_CRYPTOSYM_SW                             /**< Crypto Symbols SW Component is included. */

#define NXPBUILD__PH_CRYPTORNG_SW                             /**< Crypto RNG SW Component is included. */

/*********************************************************************************************************************************************************************************/

#ifdef NXPBUILD__PHPAL_FELICA_SW
#define NXPBUILD__PHAL_FELICA_SW                            /**< AL FeliCa SW Component is included */
#endif /* NXPBUILD__PHPAL_FELICA_SW */

#ifdef NXPBUILD__PHPAL_MIFARE_SW
#if defined(NXPBUILD__PH_KEYSTORE_SW) || defined(NXPBUILD__PH_KEYSTORE_RC663)
#define NXPBUILD__PHAL_MFC_SW                           /**< AL MIFARE Classic contactless IC SW Component is included */
#endif
#define NXPBUILD__PHAL_MFUL_SW                              /**< AL Mifare Ultrlight SW Component is included */
#define NXPBUILD__PHAL_MFDF_SW                              /**< AL Mifare DesFire SW Component is included */
//#define NXPBUILD__PHAL_MFNTAG42XDNA_SW                    /**< AL MIFARE Prime Ntag42XDna contactless IC SW Component is included */
#endif /* NXPBUILD__PHPAL_MIFARE_SW */

#define NXPBUILD__PHAL_T1T_SW                                   /**< AL Type T1 Tag SW Component is included */

#ifdef NXPBUILD__PHPAL_SLI15693_SW
#define NXPBUILD__PHAL_ICODE_SW                            /**< AL ICODE SW COMPONENT is included */
#endif /* NXPBUILD__PHPAL_SLI15693_SW */

#ifdef NXPBUILD__PHAL_T1T_SW
#define NXPBUILD__PHAL_TOP_T1T_SW                          /**< AL TOP T1T Tag SW Component is included */
#endif /* NXPBUILD__PHAL_T1T_SW */
#ifdef NXPBUILD__PHAL_MFUL_SW
#define NXPBUILD__PHAL_TOP_T2T_SW                          /**< AL TOP T2T Tag SW Component is included */
#endif /* NXPBUILD__PHAL_MFUL_SW*/
#ifdef NXPBUILD__PHAL_FELICA_SW
#define NXPBUILD__PHAL_TOP_T3T_SW                          /**< AL TOP T3T Tag SW Component is included */
#endif /* NXPBUILD__PHAL_FELICA_SW*/
#ifdef NXPBUILD__PHPAL_MIFARE_SW
#define NXPBUILD__PHAL_TOP_T4T_SW                          /**< AL TOP T4T Tag SW Component is included */
#endif /* NXPBUILD__PHPAL_MIFARE_SW*/
#ifdef NXPBUILD__PHAL_ICODE_SW
#define NXPBUILD__PHAL_TOP_T5T_SW                          /**< AL TOP T5T Tag SW Component is included */
#endif /* NXPBUILD__PHAL_ICODE_SW*/
#ifdef NXPBUILD__PHPAL_I14443P3A_SW
#define NXPBUILD__PHAL_TOP_MFC_SW                          /**< AL TOP MFC Tag SW Component is included */
#endif /* NXPBUILD__PHPAL_I14443P3A_SW*/

#if defined(NXPBUILD__PHAL_TOP_T1T_SW) || \
    defined(NXPBUILD__PHAL_TOP_T2T_SW) || \
    defined(NXPBUILD__PHAL_TOP_T3T_SW) || \
    defined(NXPBUILD__PHAL_TOP_T4T_SW) || \
    defined(NXPBUILD__PHAL_TOP_T5T_SW) || \
    defined(NXPBUILD__PHAL_TOP_MFC_SW)
#define NXPBUILD__PHAL_TOP_SW                              /**< AL for TagOps Mapping SW Component is included.Required for NDEF operations */
#endif

#ifdef NXPBUILD__PHPAL_I18000P3M3_SW
#define NXPBUILD__PHAL_I18000P3M3_SW                        /**< AL ISO18000p3m3 SW Component is included */
#endif /* NXPBUILD__PHPAL_I18000P3M3_SW */

#ifdef NXPBUILD__PHPAL_I14443P4MC_SW
#if !defined(PH_OSAL_NULLOS)
#define NXPBUILD__PHCE_T4T_SW                           /**< AL HCE T2AT SW Component is included */
//#define NXPBUILD__PHCE_T4T_PROPRIETARY                /**< SRC to handle HCE T4AT Proprietary Commands is included */
/*#define NXPBUILD__PHCE_T4T_EXT_NDEF */                /**< SRC to handle Extended NDEF Support as per T4T spec 3.0 is included */
#endif
#endif /* NXPBUILD__PHPAL_I14443P4MC_SW */

/* LLCP Components */
#if defined(NXPBUILD__PHPAL_I18092MPI_SW) || defined(NXPBUILD__PHPAL_I18092MT_SW)
#if !defined(PH_OSAL_NULLOS)
//      #define NXPBUILD__PHLN_LLCP_SW                           /**< Link LLCP SW Component is included */
#endif
#endif

/* SNEP components */
#ifdef NXPBUILD__PHLN_LLCP_SW
#define NXPBUILD__PHNP_SNEP_SW                               /**< Protocol SNEP SW Component is included */
#endif /* NXPBUILD__PHLN_LLCP_SW */

/* Enable/disable Debugging */
#define NXPBUILD__PH_DEBUG                                   /**< TODO: To be checked if required */
//#define NXPBUILD__PH_LOG
/** @}
* end of ph_NxpBuild
*/

#endif /* PH_NXPBUILD_APP_H_INC */
