## Example Summary

This application demonstrates how to use the `ti.sysbios.knl.Clock` module to
construct one-shot and periodic Clock instances. Clock Instances are
essentially functions that run after a certain number of Clock ticks.

## Example Usage

* Run the application, the two Clock instances will each print messages to the
console with their corresponding timeout parameters.

## Application Design Details

* The application is designed to run two Clock instances such that the peridoc
instance runs twice while the one-shot is only triggered once. As such, the
functions that are passed into the Clock instances, `clk0Fxn` and `clk1Fxn`
respectively are run when the instance reaches its' prescribed timeout.

> To configure the Clock module, such as to set the systems `tickPeriod` in
microseconds, please see the applications' corresponding configuration file
*clock.cfg*.

> The Clock instances periods' are scaled to decrease the dependency on the
accuracy of the hardware clock used to base the system `tickPeriod` of off.
As such, the expected output may vary between devices using 1000 vs 10
microsecond periods.

Ex)

For CC13xx/CC26xx devices with a system `tickPeriod` of 10 microseconds,
the clock instances will trigger after approximately 500, 1000 and
1100 system ticks.

For the MSP432 running with a system `tickPeriod` of 1000 microseconds, the
clock instances will trigger after approximately 5, 10 and 11 system ticks.

## References
* For GNU and IAR users, please read the following website for details
  about enabling [semi-hosting](http://processors.wiki.ti.com/index.php/TI-RTOS_Examples_SemiHosting)
  in order to view console output.

* For more help, search either the SYS/BIOS User Guide or the TI-RTOS
Getting Started Guide within your TI-RTOS installation.
