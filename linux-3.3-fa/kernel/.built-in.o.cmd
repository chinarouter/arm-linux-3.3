cmd_kernel/built-in.o :=  /opt/gm8136/toolchain_gnueabi-4.4.0_ARMv5TE/usr/bin/arm-unknown-linux-uclibcgnueabi-ld -EL    -r -o kernel/built-in.o kernel/fork.o kernel/exec_domain.o kernel/panic.o kernel/printk.o kernel/cpu.o kernel/exit.o kernel/itimer.o kernel/time.o kernel/softirq.o kernel/resource.o kernel/sysctl.o kernel/sysctl_binary.o kernel/capability.o kernel/ptrace.o kernel/timer.o kernel/user.o kernel/signal.o kernel/sys.o kernel/kmod.o kernel/workqueue.o kernel/pid.o kernel/rcupdate.o kernel/extable.o kernel/params.o kernel/posix-timers.o kernel/kthread.o kernel/wait.o kernel/kfifo.o kernel/sys_ni.o kernel/posix-cpu-timers.o kernel/mutex.o kernel/hrtimer.o kernel/rwsem.o kernel/nsproxy.o kernel/srcu.o kernel/semaphore.o kernel/notifier.o kernel/ksysfs.o kernel/cred.o kernel/async.o kernel/range.o kernel/groups.o kernel/sched/built-in.o kernel/power/built-in.o kernel/sysctl_check.o kernel/time/built-in.o kernel/futex.o kernel/rtmutex.o kernel/up.o kernel/uid16.o kernel/module.o kernel/kallsyms.o kernel/acct.o kernel/utsname.o kernel/user_namespace.o kernel/pid_namespace.o kernel/configs.o kernel/hung_task.o kernel/irq/built-in.o kernel/rcutiny.o kernel/utsname_sysctl.o kernel/elfcore.o kernel/irq_work.o kernel/events/built-in.o ; scripts/mod/modpost kernel/built-in.o