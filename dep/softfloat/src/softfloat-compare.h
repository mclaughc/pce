/*============================================================================
This C header file is part of the SoftFloat IEC/IEEE Floating-point Arithmetic
Package, Release 2b.

Written by John R. Hauser.  This work was made possible in part by the
International Computer Science Institute, located at Suite 600, 1947 Center
Street, Berkeley, California 94704.  Funding was partially provided by the
National Science Foundation under grant MIP-9311980.  The original version
of this code was written as part of a project to build a fixed-point vector
processor in collaboration with the University of California at Berkeley,
overseen by Profs. Nelson Morgan and John Wawrzynek.  More information
is available through the Web page `http://www.cs.berkeley.edu/~jhauser/
arithmetic/SoftFloat.html'.

THIS SOFTWARE IS DISTRIBUTED AS IS, FOR FREE.  Although reasonable effort has
been made to avoid it, THIS SOFTWARE MAY CONTAIN FAULTS THAT WILL AT TIMES
RESULT IN INCORRECT BEHAVIOR.  USE OF THIS SOFTWARE IS RESTRICTED TO PERSONS
AND ORGANIZATIONS WHO CAN AND WILL TAKE FULL RESPONSIBILITY FOR ALL LOSSES,
COSTS, OR OTHER PROBLEMS THEY INCUR DUE TO THE SOFTWARE, AND WHO FURTHERMORE
EFFECTIVELY INDEMNIFY JOHN HAUSER AND THE INTERNATIONAL COMPUTER SCIENCE
INSTITUTE (possibly via similar legal warning) AGAINST ALL LOSSES, COSTS, OR
OTHER PROBLEMS INCURRED BY THEIR CUSTOMERS AND CLIENTS DUE TO THE SOFTWARE.

Derivative works are acceptable, even for commercial purposes, so long as
(1) the source code for the derivative work includes prominent notice that
the work is derivative, and (2) the source code includes prominent notice with
these four paragraphs for those parts of this code that are retained.
=============================================================================*/

/*============================================================================
 * Adapted for Bochs (x86 achitecture simulator) by
 *            Stanislav Shwartsman [sshwarts at sourceforge net]
 * ==========================================================================*/

#ifndef _SOFTFLOAT_COMPARE_H_
#define _SOFTFLOAT_COMPARE_H_

#include "softfloat.h"

// ======= float32 ======= //

typedef int (*float32_compare_method)(float32, float32, float_status_t &status);

// 0x00
inline int float32_eq_ordered_quiet(float32 a, float32 b, float_status_t &status)
{
   int relation = float32_compare_quiet(a, b, status);
   return (relation == float_relation_equal);
}

// 0x01
inline int float32_lt_ordered_signalling(float32 a, float32 b, float_status_t &status)
{
   int relation = float32_compare(a, b, status);
   return (relation == float_relation_less);
}

// 0x02
inline int float32_le_ordered_signalling(float32 a, float32 b, float_status_t &status)
{
   int relation = float32_compare(a, b, status);
   return (relation == float_relation_less) || (relation == float_relation_equal);
}

// 0x03
inline int float32_unordered_quiet(float32 a, float32 b, float_status_t &status)
{
   int relation = float32_compare_quiet(a, b, status);
   return (relation == float_relation_unordered);
}

// 0x04
inline int float32_neq_unordered_quiet(float32 a, float32 b, float_status_t &status)
{
   int relation = float32_compare_quiet(a, b, status);
   return (relation != float_relation_equal);
}

// 0x05
inline int float32_nlt_unordered_signalling(float32 a, float32 b, float_status_t &status)
{
   int relation = float32_compare(a, b, status);
   return (relation != float_relation_less);
}

// 0x06
inline int float32_nle_unordered_signalling(float32 a, float32 b, float_status_t &status)
{
   int relation = float32_compare(a, b, status);
   return (relation != float_relation_less) && (relation != float_relation_equal);
}

// 0x07
inline int float32_ordered_quiet(float32 a, float32 b, float_status_t &status)
{
   int relation = float32_compare_quiet(a, b, status);
   return (relation != float_relation_unordered);
}

// 0x08
inline int float32_eq_unordered_quiet(float32 a, float32 b, float_status_t &status)
{
   int relation = float32_compare_quiet(a, b, status);
   return (relation == float_relation_equal) || (relation == float_relation_unordered);
}

// 0x09
inline int float32_nge_unordered_signalling(float32 a, float32 b, float_status_t &status)
{
   int relation = float32_compare(a, b, status);
   return (relation == float_relation_less) || (relation == float_relation_unordered);
}

// 0x0a
inline int float32_ngt_unordered_signalling(float32 a, float32 b, float_status_t &status)
{
   int relation = float32_compare(a, b, status);
   return (relation != float_relation_greater);
}

// 0x0b
inline int float32_false_quiet(float32 a, float32 b, float_status_t &status)
{
   float32_compare_quiet(a, b, status);
   return 0;
}

// 0x0c
inline int float32_neq_ordered_quiet(float32 a, float32 b, float_status_t &status)
{
   int relation = float32_compare_quiet(a, b, status);
   return (relation != float_relation_equal) && (relation != float_relation_unordered);
}

// 0x0d
inline int float32_ge_ordered_signalling(float32 a, float32 b, float_status_t &status)
{
   int relation = float32_compare(a, b, status);
   return (relation == float_relation_greater) || (relation == float_relation_equal);
}

// 0x0e
inline int float32_gt_ordered_signalling(float32 a, float32 b, float_status_t &status)
{
   int relation = float32_compare(a, b, status);
   return (relation == float_relation_greater);
}

// 0x0f
inline int float32_true_quiet(float32 a, float32 b, float_status_t &status)
{
   float32_compare_quiet(a, b, status);
   return 1;
}

// 0x10
inline int float32_eq_ordered_signalling(float32 a, float32 b, float_status_t &status)
{
   int relation = float32_compare(a, b, status);
   return (relation == float_relation_equal);
}

// 0x11
inline int float32_lt_ordered_quiet(float32 a, float32 b, float_status_t &status)
{
   int relation = float32_compare_quiet(a, b, status);
   return (relation == float_relation_less);
}

// 0x12
inline int float32_le_ordered_quiet(float32 a, float32 b, float_status_t &status)
{
   int relation = float32_compare_quiet(a, b, status);
   return (relation == float_relation_less) || (relation == float_relation_equal);
}

// 0x13
inline int float32_unordered_signalling(float32 a, float32 b, float_status_t &status)
{
   int relation = float32_compare(a, b, status);
   return (relation == float_relation_unordered);
}

// 0x14
inline int float32_neq_unordered_signalling(float32 a, float32 b, float_status_t &status)
{
   int relation = float32_compare(a, b, status);
   return (relation != float_relation_equal);
}

// 0x15
inline int float32_nlt_unordered_quiet(float32 a, float32 b, float_status_t &status)
{
   int relation = float32_compare_quiet(a, b, status);
   return (relation != float_relation_less);
}

// 0x16
inline int float32_nle_unordered_quiet(float32 a, float32 b, float_status_t &status)
{
   int relation = float32_compare_quiet(a, b, status);
   return (relation != float_relation_less) && (relation != float_relation_equal);
}

// 0x17
inline int float32_ordered_signalling(float32 a, float32 b, float_status_t &status)
{
   int relation = float32_compare(a, b, status);
   return (relation != float_relation_unordered);
}

// 0x18
inline int float32_eq_unordered_signalling(float32 a, float32 b, float_status_t &status)
{
   int relation = float32_compare(a, b, status);
   return (relation == float_relation_equal) || (relation == float_relation_unordered);
}

// 0x19
inline int float32_nge_unordered_quiet(float32 a, float32 b, float_status_t &status)
{
   int relation = float32_compare_quiet(a, b, status);
   return (relation == float_relation_less) || (relation == float_relation_unordered);
}

// 0x1a
inline int float32_ngt_unordered_quiet(float32 a, float32 b, float_status_t &status)
{
   int relation = float32_compare_quiet(a, b, status);
   return (relation != float_relation_greater);
}

// 0x1b
inline int float32_false_signalling(float32 a, float32 b, float_status_t &status)
{
   float32_compare(a, b, status);
   return 0;
}

// 0x1c
inline int float32_neq_ordered_signalling(float32 a, float32 b, float_status_t &status)
{
   int relation = float32_compare(a, b, status);
   return (relation != float_relation_equal) && (relation != float_relation_unordered);
}

// 0x1d
inline int float32_ge_ordered_quiet(float32 a, float32 b, float_status_t &status)
{
   int relation = float32_compare_quiet(a, b, status);
   return (relation == float_relation_greater) || (relation == float_relation_equal);
}

// 0x1e
inline int float32_gt_ordered_quiet(float32 a, float32 b, float_status_t &status)
{
   int relation = float32_compare_quiet(a, b, status);
   return (relation == float_relation_greater);
}

// 0x1f
inline int float32_true_signalling(float32 a, float32 b, float_status_t &status)
{
   float32_compare(a, b, status);
   return 1;
}

// ======= float64 ======= //

typedef int (*float64_compare_method)(float64, float64, float_status_t &status);

// 0x00
inline int float64_eq_ordered_quiet(float64 a, float64 b, float_status_t &status)
{
   int relation = float64_compare_quiet(a, b, status);
   return (relation == float_relation_equal);
}

// 0x01
inline int float64_lt_ordered_signalling(float64 a, float64 b, float_status_t &status)
{
   int relation = float64_compare(a, b, status);
   return (relation == float_relation_less);
}

// 0x02
inline int float64_le_ordered_signalling(float64 a, float64 b, float_status_t &status)
{
   int relation = float64_compare(a, b, status);
   return (relation == float_relation_less) || (relation == float_relation_equal);
}

// 0x03
inline int float64_unordered_quiet(float64 a, float64 b, float_status_t &status)
{
   int relation = float64_compare_quiet(a, b, status);
   return (relation == float_relation_unordered);
}

// 0x04
inline int float64_neq_unordered_quiet(float64 a, float64 b, float_status_t &status)
{
   int relation = float64_compare_quiet(a, b, status);
   return (relation != float_relation_equal);
}

// 0x05
inline int float64_nlt_unordered_signalling(float64 a, float64 b, float_status_t &status)
{
   int relation = float64_compare(a, b, status);
   return (relation != float_relation_less);
}

// 0x06
inline int float64_nle_unordered_signalling(float64 a, float64 b, float_status_t &status)
{
   int relation = float64_compare(a, b, status);
   return (relation != float_relation_less) && (relation != float_relation_equal);
}

// 0x07
inline int float64_ordered_quiet(float64 a, float64 b, float_status_t &status)
{
   int relation = float64_compare_quiet(a, b, status);
   return (relation != float_relation_unordered);
}

// 0x08
inline int float64_eq_unordered_quiet(float64 a, float64 b, float_status_t &status)
{
   int relation = float64_compare_quiet(a, b, status);
   return (relation == float_relation_equal) || (relation == float_relation_unordered);
}

// 0x09
inline int float64_nge_unordered_signalling(float64 a, float64 b, float_status_t &status)
{
   int relation = float64_compare(a, b, status);
   return (relation == float_relation_less) || (relation == float_relation_unordered);
}

// 0x0a
inline int float64_ngt_unordered_signalling(float64 a, float64 b, float_status_t &status)
{
   int relation = float64_compare(a, b, status);
   return (relation != float_relation_greater);
}

// 0x0b
inline int float64_false_quiet(float64 a, float64 b, float_status_t &status)
{
   float64_compare_quiet(a, b, status);
   return 0;
}

// 0x0c
inline int float64_neq_ordered_quiet(float64 a, float64 b, float_status_t &status)
{
   int relation = float64_compare_quiet(a, b, status);
   return (relation != float_relation_equal) && (relation != float_relation_unordered);
}

// 0x0d
inline int float64_ge_ordered_signalling(float64 a, float64 b, float_status_t &status)
{
   int relation = float64_compare(a, b, status);
   return (relation == float_relation_greater) || (relation == float_relation_equal);
}

// 0x0e
inline int float64_gt_ordered_signalling(float64 a, float64 b, float_status_t &status)
{
   int relation = float64_compare(a, b, status);
   return (relation == float_relation_greater);
}

// 0x0f
inline int float64_true_quiet(float64 a, float64 b, float_status_t &status)
{
   float64_compare_quiet(a, b, status);
   return 1;
}

// 0x10
inline int float64_eq_ordered_signalling(float64 a, float64 b, float_status_t &status)
{
   int relation = float64_compare(a, b, status);
   return (relation == float_relation_equal);
}

// 0x11
inline int float64_lt_ordered_quiet(float64 a, float64 b, float_status_t &status)
{
   int relation = float64_compare_quiet(a, b, status);
   return (relation == float_relation_less);
}

// 0x12
inline int float64_le_ordered_quiet(float64 a, float64 b, float_status_t &status)
{
   int relation = float64_compare_quiet(a, b, status);
   return (relation == float_relation_less) || (relation == float_relation_equal);
}

// 0x13
inline int float64_unordered_signalling(float64 a, float64 b, float_status_t &status)
{
   int relation = float64_compare(a, b, status);
   return (relation == float_relation_unordered);
}

// 0x14
inline int float64_neq_unordered_signalling(float64 a, float64 b, float_status_t &status)
{
   int relation = float64_compare(a, b, status);
   return (relation != float_relation_equal);
}

// 0x15
inline int float64_nlt_unordered_quiet(float64 a, float64 b, float_status_t &status)
{
   int relation = float64_compare_quiet(a, b, status);
   return (relation != float_relation_less);
}

// 0x16
inline int float64_nle_unordered_quiet(float64 a, float64 b, float_status_t &status)
{
   int relation = float64_compare_quiet(a, b, status);
   return (relation != float_relation_less) && (relation != float_relation_equal);
}

// 0x17
inline int float64_ordered_signalling(float64 a, float64 b, float_status_t &status)
{
   int relation = float64_compare(a, b, status);
   return (relation != float_relation_unordered);
}

// 0x18
inline int float64_eq_unordered_signalling(float64 a, float64 b, float_status_t &status)
{
   int relation = float64_compare(a, b, status);
   return (relation == float_relation_equal) || (relation == float_relation_unordered);
}

// 0x19
inline int float64_nge_unordered_quiet(float64 a, float64 b, float_status_t &status)
{
   int relation = float64_compare_quiet(a, b, status);
   return (relation == float_relation_less) || (relation == float_relation_unordered);
}

// 0x1a
inline int float64_ngt_unordered_quiet(float64 a, float64 b, float_status_t &status)
{
   int relation = float64_compare_quiet(a, b, status);
   return (relation != float_relation_greater);
}

// 0x1b
inline int float64_false_signalling(float64 a, float64 b, float_status_t &status)
{
   float64_compare(a, b, status);
   return 0;
}

// 0x1c
inline int float64_neq_ordered_signalling(float64 a, float64 b, float_status_t &status)
{
   int relation = float64_compare(a, b, status);
   return (relation != float_relation_equal) && (relation != float_relation_unordered);
}

// 0x1d
inline int float64_ge_ordered_quiet(float64 a, float64 b, float_status_t &status)
{
   int relation = float64_compare_quiet(a, b, status);
   return (relation == float_relation_greater) || (relation == float_relation_equal);
}

// 0x1e
inline int float64_gt_ordered_quiet(float64 a, float64 b, float_status_t &status)
{
   int relation = float64_compare_quiet(a, b, status);
   return (relation == float_relation_greater);
}

// 0x1f
inline int float64_true_signalling(float64 a, float64 b, float_status_t &status)
{
   float64_compare(a, b, status);
   return 1;
}

#endif
