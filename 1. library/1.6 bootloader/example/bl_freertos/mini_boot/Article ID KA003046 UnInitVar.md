**Article ID:** KA003046
**Applies To:** Arm Compiler 5, Arm Compiler 6, Keil MDK
**Confidentiality:** Customer Non-confidential

Information in this knowledgebase article applies to:

- Keil [MDK](https://www2.keil.com/mdk5) v. [4.02](http://www.keil.com/update/sw/MDK4/4.02) and later
- Keil [µVision IDE](https://www2.keil.com/mdk5/uvision) v. 4.00d and later
- [CMSIS-Pack](https://arm-software.github.io/CMSIS_5) v. [1.3.0](http://www.keil.com/dd2/pack#ARM.CMSIS) and later
- [ARM Compiler 5 (Armcc)](https://www2.keil.com/mdk5/compiler/5) v. [5.02 (build 28)](https://developer.arm.com/tools-and-software/embedded/arm-compiler/downloads/legacy-compilers#arm-compiler-5) and later
- [ARM Compiler 6 (Armclang)](https://www2.keil.com/mdk5/compiler/6) v. [6.4](https://developer.arm.com/tools-and-software/embedded/arm-compiler/downloads/version-6) and later

#### SYMPTOM

I have a section of memory that I do not want to be initialized. In my scatter file I have this region of memory marked as UNINIT. I am using __attribute__((section(""))) to place variables in the UNINIT region.

Scatter file:



```c
LR_IROM1 0x08000000 0x00080000  {    ; load region size_region
  ER_IROM1 0x08000000 0x00080000  {  ; load address = execution address
   *.o (RESET, +First)
   *(InRoot$$Sections)
   .ANY (+RO)
  }
  RW_IRAM1 0x20000000 UNINIT 0x00000100  { ;no init section
        *(NoInit)
   }
  RW_IRAM2 0x20000100 0x0000FFF0  {                ;all other rw data
        .ANY(+RW +ZI)
  }
}
```



C Code:



```c
unsigned long NI_longVar __attribute__( ( section( "NoInit") ) ) ;


int main( void )
{
  while( 1 )
  {
  }
}
```



The variable is being placed in the UNINIT region properly, however, the memory contents of the variable are still being initialized. It appears as if the UNINIT attribute in the scatter file is being ignored. Why is this?

#### CAUSE

Only ZI data in execution regions with the UNINIT attribute remains un-initialized. However any RW data in the execution regions with the UNINIT attribute does get initialized.

Arm Compiler 5, for optimization reasons, puts global ZI variables of 8 bytes or less by default into RW data sections. Since the variable in the code sample above is less than 8 bytes, it is being changed from ZI to RW which causes initialization to take place.

Arm Compiler 6 only creates ZI sections, if the section name starts with ".bss". As the section name in the example above does not start with ".bss", initialization to take place.

#### RESOLUTION

When using Arm Compiler 5, add the zero_init attribute to the variable in order to prevent the optimization. This keeps the variable as ZI data which will not be initialized.

When using Arm Compiler 6, rename the section name "NoInit" to ".bss.NoInit". This creates the variable in a ZI section, where initialization can be prevented.





```c
// Arm Compiler 5
unsigned long NI_longVar __attribute__( ( section( "NoInit"),zero_init) ) ;

// Arm Compiler 6
unsigned long NI_longVar __attribute__( ( section( ".bss.NoInit")) ) ;
//also modify one line of the scatterfile from:
//        *(NoInit)
//to:
//        *(.bss.NoInit)
```