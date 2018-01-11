// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/process/process_info.h"

#include <stdint.h>

#include "base/logging.h"
#include "base/process/internal_linux.h"
#include "base/process/process_handle.h"
#include "base/time/time.h"

#if defined(__FreeBSD__) || defined(__DragonFly__)
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/user.h>
#endif

namespace base {

// static
const Time CurrentProcessInfo::CreationTime() {
#if defined(__FreeBSD__) || defined(__DragonFly__)
  int mib[] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid() };
  struct kinfo_proc proc;
  size_t len = sizeof(struct kinfo_proc);
  if (sysctl(mib, arraysize(mib), &proc, &len, NULL, 0) < 0)
    return Time();
#if defined(__DragonFly__)
  return Time::FromTimeVal(proc.kp_start);
#else
  return Time::FromTimeVal(proc.ki_start);
#endif
#else
  ProcessHandle pid = GetCurrentProcessHandle();
  int64_t start_ticks =
      internal::ReadProcStatsAndGetFieldAsInt64(pid, internal::VM_STARTTIME);
  DCHECK(start_ticks);
  TimeDelta start_offset = internal::ClockTicksToTimeDelta(start_ticks);
  Time boot_time = internal::GetBootTime();
  DCHECK(!boot_time.is_null());
  return Time(boot_time + start_offset);
#endif
}

}  // namespace base
