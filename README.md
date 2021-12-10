# Survivor: Surviving Transient Power Failures with SRAM Data Retention
Survivor is a task-based programming and executing model for energy-harvesting battery-less devices. Survivor applications should be written as a chain of tasks. Survivor runtime library protects applications data against power interrupts with SRAM data retention feature. For more technical details, please refer to our paper [<sup>1</sup>](#refer-anchor-1).

## Programming interface
- A Survivor's task should be declared as
- A global Survivor's variable should be declared as
- A global protected variable should be accessed with
- The macro `NEXT(<next task>)` should be used to transition from one task to another.
- Survivor's scheduler is invoked with `__scheduler_run`

## Developing applications with Survivor


## Reference
<div id="refer-anchor-1"></div>
- [1] [Liu, Songran, et al. "Surviving Transient Power Failures with SRAM Data Retention." 2021 Design, Automation & Test in Europe Conference & Exhibition (DATE). IEEE, 2021.]
