/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_EXT_ASIO_WAIT_HANDLE_H_
#define incl_HPHP_EXT_ASIO_WAIT_HANDLE_H_

#include "hphp/runtime/base/base-includes.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class WaitHandle

/**
 * A wait handle is an object that describes operation that is potentially
 * asynchronous. A WaitHandle class is a base class of all such objects. There
 * are multiple types of wait handles, this is their hierarchy:
 *
 * WaitHandle                     - abstract wait handle
 *   StaticWaitHandle             - statically finished wait handle
 *   WaitableWaitHandle           - wait handle that can be waited for
 *     BlockableWaitHandle        - wait handle that can be blocked by other WH
 *       AsyncFunctionWaitHandle  - async function-based asynchronous execution
 *       GenArrayWaitHandle       - wait handle representing an array of WHs
 *       GenMapWaitHandle         - wait handle representing an Map of WHs
 *       GenVectorWaitHandle      - wait handle representing an Vector of WHs
 *       SetResultToRefWaitHandle - wait handle that sets result to reference
 *     RescheduleWaitHandle       - wait handle that reschedules execution
 *     SleepWaitHandle            - wait handle that finishes after a timeout
 *     ExternalThreadEventWaitHandle  - thread-powered asynchronous execution
 *
 * A wait handle can be either synchronously joined (waited for the operation
 * to finish) or passed in various contexts as a dependency and waited for
 * asynchronously (such as using await mechanism of async function or
 * passed as an array member of GenArrayWaitHandle).
 */
FORWARD_DECLARE_CLASS(WaitHandle);
class c_WaitHandle : public ExtObjectDataFlags<ObjectData::IsWaitHandle> {
 public:
  DECLARE_CLASS_NO_SWEEP(WaitHandle)

  enum class Kind : uint8_t {
    Static              = 0,
    AsyncFunction       = 1,
    GenArray            = 2,
    GenMap              = 3,
    GenVector           = 4,
    SetResultToRef      = 5,
    Reschedule          = 6,
    Sleep               = 7,
    ExternalThreadEvent = 8,
  };

  explicit c_WaitHandle(Class* cls = c_WaitHandle::classof())
    : ExtObjectDataFlags(cls)
    , m_resultOrException(make_tv<KindOfNull>())
  {}
  ~c_WaitHandle() {}

  void t___construct();
  static void ti_setonjoincallback(const Variant& callback);
  Object t_getwaithandle();
  void t_import();
  Variant t_join();
  bool t_isfinished();
  bool t_issucceeded();
  bool t_isfailed();
  int64_t t_getid();
  String t_getname();
  Object t_getexceptioniffailed();

 public:
  static constexpr ptrdiff_t stateOff() {
    return offsetof(c_WaitHandle, o_subclassData.u8[0]);
  }
  static constexpr ptrdiff_t resultOff() {
    return offsetof(c_WaitHandle, m_resultOrException);
  }

  static c_WaitHandle* fromCell(Cell* cell) {
    return (
        cell->m_type == KindOfObject &&
        cell->m_data.pobj->getAttribute(ObjectData::IsWaitHandle)
      ) ? static_cast<c_WaitHandle*>(cell->m_data.pobj) : nullptr;
  }
  bool isFinished() const { return getState() <= STATE_FAILED; }
  bool isSucceeded() const { return getState() == STATE_SUCCEEDED; }
  bool isFailed() const { return getState() == STATE_FAILED; }
  const Cell& getResult() const {
    assert(isSucceeded());
    return m_resultOrException;
  }
  ObjectData* getException() const {
    assert(isFailed());
    return m_resultOrException.m_data.pobj;
  }

  Kind getKind() const { return static_cast<Kind>(o_subclassData.u8[0] >> 4); }
  uint8_t getState() const { return o_subclassData.u8[0] & 0x0F; }
  static uint8_t toKindState(Kind kind, uint8_t state) {
    assert((uint8_t)kind < 0x10 && state < 0x10);
    return ((uint8_t)kind << 4) | state;
  }
  void setKindState(Kind kind, uint8_t state) {
    o_subclassData.u8[0] = toKindState(kind, state);
  }

  // The code in the TC will depend on the values of these constants.
  // See emitAwait().
  static const int8_t STATE_SUCCEEDED = 0;
  static const int8_t STATE_FAILED    = 1;

 protected:
  Cell m_resultOrException;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ASIO_WAIT_HANDLE_H_
