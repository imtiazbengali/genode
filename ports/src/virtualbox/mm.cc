/*
 * \brief  VirtualBox memory manager (MMR3)
 * \author Norman Feske
 * \date   2013-08-20
 */

/*
 * Copyright (C) 2013 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

/* Genode includes */
#include <base/printf.h>
#include <util/string.h>

/* VirtualBox includes */
#include <VBox/vmm/mm.h>
#include <VBox/vmm/cfgm.h>
#include <VBox/err.h>
#include <VBox/vmm/gmm.h>
#include <VBox/vmm/vm.h>
#include <VBox/vmm/pgm.h>
#include <iprt/err.h>
#include <iprt/mem.h>
#include <iprt/string.h>

/* libc memory allocator */
#include <libc_mem_alloc.h>

#include "util.h"


int MMR3Init(PVM pVM)
{
	PDBG("MMR3Init called, not implemented");
	return VINF_SUCCESS;
}


int MMR3InitUVM(PUVM pUVM)
{
	PDBG("MMR3InitUVM called, not implemented");
	return VINF_SUCCESS;
}


void *MMR3HeapAllocU(PUVM pUVM, MMTAG enmTag, size_t cbSize)
{
	return Libc::mem_alloc()->alloc(cbSize, Genode::log2(RTMEM_ALIGNMENT));
}


/**
 * Return alignment to be used for allocations of given tag
 */
static unsigned align_by_mmtag(MMTAG enmTag)
{
	switch (enmTag) {
	case MM_TAG_PDM_DEVICE:
	case MM_TAG_PDM_DEVICE_USER:
		return 12;
	default:
		return Genode::log2(RTMEM_ALIGNMENT);
	}
}


/**
 * Round allocation size for a given tag
 */
static size_t round_size_by_mmtag(MMTAG enmTag, size_t cb)
{
	return Genode::align_addr(cb, align_by_mmtag(enmTag));
}


void *MMR3HeapAlloc(PVM pVM, MMTAG enmTag, size_t cbSize)
{
	size_t const rounded_size = round_size_by_mmtag(enmTag, cbSize);

	void *ret = Libc::mem_alloc()->alloc(rounded_size, align_by_mmtag(enmTag));

//	PINF("MMR3HeapAlloc: enmTag=%d cbSize=0x%zx -> 0x%p (0x%zx)",
//	     enmTag, cbSize, ret, rounded_size);

	return ret;
}


void *MMR3HeapAllocZ(PVM pVM, MMTAG enmTag, size_t cbSize)
{
	void * const ret = MMR3HeapAlloc(pVM, enmTag, cbSize);

	if (ret)
		Genode::memset(ret, 0, cbSize);

	return ret;
}


int MMR3HeapAllocZEx(PVM pVM, MMTAG enmTag, size_t cbSize, void **ppv)
{
	*ppv = MMR3HeapAllocZ(pVM, enmTag, cbSize);

	return VINF_SUCCESS;
}


int MMR3HyperAllocOnceNoRel(PVM pVM, size_t cb, unsigned uAlignment,
                            MMTAG enmTag, void **ppv)
{
	unsigned const align_log2 = uAlignment ? Genode::log2(uAlignment)
	                                       : align_by_mmtag(enmTag);

	size_t const rounded_size = round_size_by_mmtag(enmTag, cb);

	void *ret = Libc::mem_alloc()->alloc(rounded_size, align_log2);
	if (ret)
		Genode::memset(ret, 0, cb);

	PINF("MMR3HyperAllocOnceNoRel: enmTag=%d align_log2=%u cb=0x%zx -> 0x%p",
	     enmTag, align_log2, cb, ret);

	*ppv = ret;

	return VINF_SUCCESS;
}


int MMR3HyperAllocOnceNoRelEx(PVM pVM, size_t cb, uint32_t uAlignment,
                              MMTAG enmTag, uint32_t fFlags, void **ppv)
{
	return MMR3HyperAllocOnceNoRel(pVM, cb, uAlignment, enmTag, ppv);
}


int MMHyperAlloc(PVM pVM, size_t cb, unsigned uAlignment, MMTAG enmTag, void **ppv)
{
	AssertRelease(align_by_mmtag(enmTag) >= uAlignment);

	*ppv = MMR3HeapAllocZ(pVM, enmTag, cb);
	return VINF_SUCCESS;
}


int MMHyperFree(PVM pVM, void *pv)
{
	Libc::mem_alloc()->free(pv);

	return VINF_SUCCESS;
}


void MMR3HeapFree(void *pv) { Libc::mem_alloc()->free(pv); }


RTR0PTR MMHyperR3ToR0(PVM pVM, RTR3PTR R3Ptr) { return (RTR0PTR)R3Ptr; }
RTRCPTR MMHyperR3ToRC(PVM pVM, RTR3PTR R3Ptr) { return to_rtrcptr(R3Ptr); }
RTR0PTR MMHyperCCToR0(PVM pVM, void *pv)      { return (RTR0PTR)pv; }
RTRCPTR MMHyperCCToRC(PVM pVM, void *pv)      { return to_rtrcptr(pv); }


uint64_t MMR3PhysGetRamSize(PVM pVM)
{
	PDBG("MMR3PhysGetRamSize called, return 0");

	/* when called from REMR3Init, it is expected to return 0 */
	return 0;
}


int MMR3HyperMapHCPhys(PVM pVM, void *pvR3, RTR0PTR pvR0, RTHCPHYS HCPhys,
                       size_t cb, const char *pszDesc, PRTGCPTR pGCPtr)
{
	PDBG("pszDesc=%s", pszDesc);

	*pGCPtr = (RTGCPTR)HCPhys;

	return VINF_SUCCESS;
}


int MMR3HyperReserve(PVM pVM, unsigned cb, const char *pszDesc, PRTGCPTR pGCPtr)
{
	PINF("MMR3HyperReserve: cb=0x%x, pszDesc=%s", cb, pszDesc);

	return VINF_SUCCESS;
}


int MMR3HyperMapMMIO2(PVM pVM, PPDMDEVINS pDevIns, uint32_t iRegion,
                      RTGCPHYS off, RTGCPHYS cb, const char *pszDesc,
                      PRTRCPTR pRCPtr)
{
	PLOG("MMR3HyperMapMMIO2: pszDesc=%s iRegion=%u off=0x%lx cb=0x%zx",
	     pszDesc, iRegion, (long)off, (size_t)cb);

	

	return VINF_SUCCESS;
}


/*
 * Based on 'VBox/VMM/VMMR3/MM.cpp'
 */
int MMR3InitPaging(PVM pVM)
{
    /*
     * Query the CFGM values.
     */
    int rc;
    PCFGMNODE pMMCfg = CFGMR3GetChild(CFGMR3GetRoot(pVM), "MM");
    if (!pMMCfg)
    {
        rc = CFGMR3InsertNode(CFGMR3GetRoot(pVM), "MM", &pMMCfg);
        AssertRCReturn(rc, rc);
    }

    /** @cfgm{RamSize, uint64_t, 0, 16TB, 0}
     * Specifies the size of the base RAM that is to be set up during
     * VM initialization.
     */
    uint64_t cbRam;
    rc = CFGMR3QueryU64(CFGMR3GetRoot(pVM), "RamSize", &cbRam);
    if (rc == VERR_CFGM_VALUE_NOT_FOUND)
        cbRam = 0;
    else
        AssertMsgRCReturn(rc, ("Configuration error: Failed to query integer \"RamSize\", rc=%Rrc.\n", rc), rc);
    cbRam &= X86_PTE_PAE_PG_MASK;

    /** @cfgm{RamHoleSize, uint32_t, 0, 4032MB, 512MB}
     * Specifies the size of the memory hole. The memory hole is used
     * to avoid mapping RAM to the range normally used for PCI memory regions.
     * Must be aligned on a 4MB boundary. */
    uint32_t cbRamHole;
    rc = CFGMR3QueryU32Def(CFGMR3GetRoot(pVM), "RamHoleSize", &cbRamHole, MM_RAM_HOLE_SIZE_DEFAULT);
    uint64_t const offRamHole = _4G - cbRamHole;

    /*
     * Make the initial memory reservation with GMM.
     */
    PDBG("GMMR3InitialReservation missing");

    /*
     * If RamSize is 0 we're done now.
     */
    if (cbRam < PAGE_SIZE)
    {
        Log(("MM: No RAM configured\n"));
        return VINF_SUCCESS;
    }

    /*
     * Setup the base ram (PGM).
     */
    if (cbRam > offRamHole)
    {
        rc = PGMR3PhysRegisterRam(pVM, 0, offRamHole, "Base RAM");
        if (RT_SUCCESS(rc))
            rc = PGMR3PhysRegisterRam(pVM, _4G, cbRam - offRamHole, "Above 4GB Base RAM");
    }
    else
        rc = PGMR3PhysRegisterRam(pVM, 0, RT_MIN(cbRam, offRamHole), "Base RAM");

    LogFlow(("MMR3InitPaging: returns %Rrc\n", rc));
    return rc;
}


char * MMR3HeapStrDup(PVM pVM, MMTAG enmTag, const char *psz)
{
    size_t cch = strlen(psz) + 1;
    char *pszDup = (char *)MMR3HeapAllocU(pVM->pUVM, enmTag, cch);
    if (pszDup)
        memcpy(pszDup, psz, cch);

	return pszDup;
}


char * MMR3HeapAPrintfVU(PUVM pUVM, MMTAG enmTag, const char *pszFormat, va_list va)
{
    /*
     * The lazy bird way.
     */
    char *psz;
    int cch = RTStrAPrintfV(&psz, pszFormat, va);
    if (cch < 0)
        return NULL;
    Assert(psz[cch] == '\0');
    char *pszRet = (char *)MMR3HeapAllocU(pUVM, enmTag, cch + 1);
    if (pszRet)
        memcpy(pszRet, psz, cch + 1);
    RTStrFree(psz);
    return pszRet;
}


extern "C" {

char * MMR3HeapAPrintf(PVM pVM, MMTAG enmTag, const char *pszFormat, ...)
{
    va_list va;
    va_start(va, pszFormat);
    char *psz = MMR3HeapAPrintfVU(pVM->pUVM, enmTag, pszFormat, va);
    va_end(va);
    return psz;
}

}
