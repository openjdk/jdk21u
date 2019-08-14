/*
 * Copyright (c) 2006, 2010, Oracle and/or its affiliates. All rights reserved.
 * Copyright (c) 2014, Red Hat Inc. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#include "precompiled.hpp"
#include "runtime/os.hpp"
#include "vm_version_aarch64.hpp"

#if defined (__FreeBSD__)
#include <machine/armreg.h>
#include <machine/elf.h>
#endif

#ifndef HWCAP_ASIMD
#define HWCAP_ASIMD (1<<1)
#endif

#ifndef HWCAP_AES
#define HWCAP_AES   (1<<3)
#endif

#ifndef HWCAP_PMULL
#define HWCAP_PMULL (1<<4)
#endif

#ifndef HWCAP_SHA1
#define HWCAP_SHA1  (1<<5)
#endif

#ifndef HWCAP_SHA2
#define HWCAP_SHA2  (1<<6)
#endif

#ifndef HWCAP_CRC32
#define HWCAP_CRC32 (1<<7)
#endif

#ifndef HWCAP_ATOMICS
#define HWCAP_ATOMICS (1<<8)
#endif

#ifndef ID_AA64PFR0_AdvSIMD_SHIFT
#define ID_AA64PFR0_AdvSIMD_SHIFT 20
#endif

#ifndef ID_AA64PFR0_AdvSIMD
#define ID_AA64PFR0_AdvSIMD(x) ((x) & (UL(0xf) << ID_AA64PFR0_AdvSIMD_SHIFT))
#endif

#ifndef ID_AA64PFR0_AdvSIMD_IMPL
#define ID_AA64PFR0_AdvSIMD_IMPL (UL(0x0) << ID_AA64PFR0_AdvSIMD_SHIFT)
#endif

#ifndef ID_AA64PFR0_AdvSIMD_HP
#define ID_AA64PFR0_AdvSIMD_HP (UL(0x1) << ID_AA64PFR0_AdvSIMD_SHIFT)
#endif

#define	CPU_IMPL_ARM		0x41
#define	CPU_IMPL_BROADCOM	0x42
#define	CPU_IMPL_CAVIUM		0x43
#define	CPU_IMPL_DEC		0x44
#define	CPU_IMPL_INFINEON	0x49
#define	CPU_IMPL_FREESCALE	0x4D
#define	CPU_IMPL_NVIDIA		0x4E
#define	CPU_IMPL_APM		0x50
#define	CPU_IMPL_QUALCOMM	0x51
#define	CPU_IMPL_MARVELL	0x56
#define	CPU_IMPL_INTEL		0x69

/* ARM Part numbers */
#define	CPU_PART_FOUNDATION	0xD00
#define	CPU_PART_CORTEX_A35	0xD04
#define	CPU_PART_CORTEX_A53	0xD03
#define	CPU_PART_CORTEX_A55	0xD05
#define	CPU_PART_CORTEX_A57	0xD07
#define	CPU_PART_CORTEX_A72	0xD08
#define	CPU_PART_CORTEX_A73	0xD09
#define	CPU_PART_CORTEX_A75	0xD0A

/* Cavium Part numbers */
#define	CPU_PART_THUNDERX	0x0A1
#define	CPU_PART_THUNDERX_81XX	0x0A2
#define	CPU_PART_THUNDERX_83XX	0x0A3
#define	CPU_PART_THUNDERX2	0x0AF

#define	CPU_REV_THUNDERX_1_0	0x00
#define	CPU_REV_THUNDERX_1_1	0x01

#define	CPU_REV_THUNDERX2_0	0x00

#define	CPU_IMPL(midr)	(((midr) >> 24) & 0xff)
#define	CPU_PART(midr)	(((midr) >> 4) & 0xfff)
#define	CPU_VAR(midr)	(((midr) >> 20) & 0xf)
#define	CPU_REV(midr)	(((midr) >> 0) & 0xf)

struct cpu_desc {
	u_int		cpu_impl;
	u_int		cpu_part_num;
	u_int		cpu_variant;
	u_int		cpu_revision;
	const char	*cpu_impl_name;
	const char	*cpu_part_name;
};

struct cpu_parts {
	u_int		part_id;
	const char	*part_name;
};
#define	CPU_PART_NONE	{ 0, "Unknown Processor" }

struct cpu_implementers {
	u_int			impl_id;
	const char		*impl_name;
	/*
	 * Part number is implementation defined
	 * so each vendor will have its own set of values and names.
	 */
	const struct cpu_parts	*cpu_parts;
};
#define	CPU_IMPLEMENTER_NONE	{ 0, "Unknown Implementer", cpu_parts_none }

/*
 * Per-implementer table of (PartNum, CPU Name) pairs.
 */
/* ARM Ltd. */
static const struct cpu_parts cpu_parts_arm[] = {
	{ CPU_PART_FOUNDATION, "Foundation-Model" },
	{ CPU_PART_CORTEX_A35, "Cortex-A35" },
	{ CPU_PART_CORTEX_A53, "Cortex-A53" },
	{ CPU_PART_CORTEX_A55, "Cortex-A55" },
	{ CPU_PART_CORTEX_A57, "Cortex-A57" },
	{ CPU_PART_CORTEX_A72, "Cortex-A72" },
	{ CPU_PART_CORTEX_A73, "Cortex-A73" },
	{ CPU_PART_CORTEX_A75, "Cortex-A75" },
	CPU_PART_NONE,
};
/* Cavium */
static const struct cpu_parts cpu_parts_cavium[] = {
	{ CPU_PART_THUNDERX, "ThunderX" },
	{ CPU_PART_THUNDERX2, "ThunderX2" },
	CPU_PART_NONE,
};

/* Unknown */
static const struct cpu_parts cpu_parts_none[] = {
	CPU_PART_NONE,
};

/*
 * Implementers table.
 */
const struct cpu_implementers cpu_implementers[] = {
	{ CPU_IMPL_ARM,		"ARM",		cpu_parts_arm },
	{ CPU_IMPL_BROADCOM,	"Broadcom",	cpu_parts_none },
	{ CPU_IMPL_CAVIUM,	"Cavium",	cpu_parts_cavium },
	{ CPU_IMPL_DEC,		"DEC",		cpu_parts_none },
	{ CPU_IMPL_INFINEON,	"IFX",		cpu_parts_none },
	{ CPU_IMPL_FREESCALE,	"Freescale",	cpu_parts_none },
	{ CPU_IMPL_NVIDIA,	"NVIDIA",	cpu_parts_none },
	{ CPU_IMPL_APM,		"APM",		cpu_parts_none },
	{ CPU_IMPL_QUALCOMM,	"Qualcomm",	cpu_parts_none },
	{ CPU_IMPL_MARVELL,	"Marvell",	cpu_parts_none },
	{ CPU_IMPL_INTEL,	"Intel",	cpu_parts_none },
	CPU_IMPLEMENTER_NONE,
};

unsigned long VM_Version::os_get_processor_features() {
  struct cpu_desc cpu_desc[1];
  struct cpu_desc user_cpu_desc;
  unsigned long auxv = 0;
  uint64_t id_aa64isar0, id_aa64pfr0;

  uint32_t midr;
  uint32_t impl_id;
  uint32_t part_id;
  uint32_t cpu = 0;
  size_t i;
  const struct cpu_parts *cpu_partsp = NULL;

  midr = READ_SPECIALREG(midr_el1);

  impl_id = CPU_IMPL(midr);
  for (i = 0; i < nitems(cpu_implementers); i++) {
    if (impl_id == cpu_implementers[i].impl_id ||
      cpu_implementers[i].impl_id == 0) {
      cpu_desc[cpu].cpu_impl = impl_id;
      cpu_desc[cpu].cpu_impl_name = cpu_implementers[i].impl_name;
      cpu_partsp = cpu_implementers[i].cpu_parts;
      break;
    }
  }

  part_id = CPU_PART(midr);
  for (i = 0; &cpu_partsp[i] != NULL; i++) {
    if (part_id == cpu_partsp[i].part_id || cpu_partsp[i].part_id == 0) {
      cpu_desc[cpu].cpu_part_num = part_id;
      cpu_desc[cpu].cpu_part_name = cpu_partsp[i].part_name;
      break;
    }
  }

  cpu_desc[cpu].cpu_revision = CPU_REV(midr);
  cpu_desc[cpu].cpu_variant = CPU_VAR(midr);

  _cpu = cpu_desc[cpu].cpu_impl;
  _variant = cpu_desc[cpu].cpu_variant;
  _model = cpu_desc[cpu].cpu_part_num;
  _revision = cpu_desc[cpu].cpu_revision;

  id_aa64isar0 = READ_SPECIALREG(ID_AA64ISAR0_EL1);
  id_aa64pfr0 = READ_SPECIALREG(ID_AA64PFR0_EL1);

  if (ID_AA64ISAR0_AES(id_aa64isar0) == ID_AA64ISAR0_AES_BASE) {
    auxv = auxv | HWCAP_AES;
  }

  if (ID_AA64ISAR0_AES(id_aa64isar0) == ID_AA64ISAR0_AES_PMULL) {
    auxv = auxv | HWCAP_PMULL;
  }

  if (ID_AA64ISAR0_SHA1(id_aa64isar0) == ID_AA64ISAR0_SHA1_BASE) {
    auxv = auxv | HWCAP_SHA1;
  }

  if (ID_AA64ISAR0_SHA2(id_aa64isar0) == ID_AA64ISAR0_SHA2_BASE) {
    auxv = auxv | HWCAP_SHA2;
  }

  if (ID_AA64ISAR0_CRC32(id_aa64isar0) == ID_AA64ISAR0_CRC32_BASE) {
    auxv = auxv | HWCAP_CRC32;
  }

  if (ID_AA64PFR0_AdvSIMD(id_aa64pfr0) == ID_AA64PFR0_AdvSIMD_IMPL || \
      ID_AA64PFR0_AdvSIMD(id_aa64pfr0) == ID_AA64PFR0_AdvSIMD_HP ) {
    auxv = auxv | HWCAP_ASIMD;
  }

  return auxv;
}
