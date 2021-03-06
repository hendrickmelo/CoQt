#ifndef FIBER_H
#define FIBER_H

#include <QObject>
#include <functional>
#include <QSharedPointer>
#include <QWeakPointer>
#include "functional"
#include "qxtpimpl.h"
#include "wakecondition.h"
#include "context.h"

namespace CoQt
{
class WakeCondition;

//Createing a Fiber
//
//Use the createFiber function to create a new fiber. A fiber consists of a
//main function, similar to a thread. This function is run immediately, and
//the fiber is finished when the function returns. Before returning, the fiber
//may yeild at any time. Once the fiber yields for the first time, createFiber()
//returns. At this time the fiber has not yet returned, and will be in the idle
//state. The fiber can be run again with a call to the wake() function. Once woken
//the fiber continues from the point it which it last yielded. It will then run untill
//the fiber returns, or yields once more. Once this occurs the wake() function returns.

class FiberPrivate;

class Fiber : public QObject
{
    Q_OBJECT
    QXT_DECLARE_PRIVATE(Fiber)

    //Private constructor use CoQt::createFirber to create fiber objects
    explicit Fiber(std::function<void()> func, QObject *parent = 0);

    friend QSharedPointer<Fiber> createFiber(const std::function<void ()> &);
public:
    enum FiberState
    {
        FiberIdle,          //The fiber has not started
        FiberRunning,       //The fiber is currently running, if fibers are nested, it may not be the active fiber
        FiberWaiting,       //The fiber has yelded, and is waiting to be woken
        FiberFinished       //The fiber has finished
    };


    ~Fiber();

    //State functions
    FiberState getState();
    bool isRunning();
    bool isWaiting();
    bool isFinished();

    //Yield functions
    //
    //These functions will cause the fiber to pause, and control flow to pass to
    //the function that called wake() on this fuber.
    //
    //These functions are static, and act on the currently running fiber. If there
    //is no current fiber, the bahavior is undefined

    //Yield the fiber, and wake on the next scheduler tick
    static void yield();

    //Yield the fiber, and wake after iMs milliseconds. if iMs is less than one
    //scheduler tick, than the scheduler will wait a full tick before waking the
    //fiber
    static void yield(int iMs);

    //Yield the fiber and call func() every iPollInterval MS. If MS is 0, ore
    //is less than one scheduler tick, the function will be called on the next
    //tick. If the the function returns true, the fiber is woken, otherwise
    //the function is run again on the next poll interval
    static void yield(std::function<bool()> func, int iPollInterval = 0);

    //Same as above, but takes an object and function pointer
    template <class T>
    static void yield(T *object,void (T::*pFunc)(), int iPollInterval = 0)
    {
        yeild(std::bind(pFunc, object), iPollInterval);
    }

    //Yield the fiber untill the specified QFuture is finished or canceled
    //one the fiber is woken, the QFuture is returned so results can be
    //retrieved. This allows a fiber to yield and wait for the outcome of
    //any function in QtConcurrent that returns a QFuture.
    //QtConcurrent::run() for example would allow a synchronous task to be
    //run on another thread, and its' results to be retrieved when the task
    //completes
    template <class T>
    static QFuture<T> yield(QFuture<T> future)
    {
        if(!context()->curFiber())
           return future;

        QFutureWakeCondition<T> waker(&future, context()->curFiber());
        yieldForever();

        return future;
    }

    //Yield the fiber, and wake when the specified signal is emitted
    static void yield(QObject *obj, const char *signal);

    //Yeild the fiber and use the specified WakeCondition object to determine
    //when the fiber should be woken
    static void yield(WakeCondition *pCondition);

    //Yield the fiber until the wake() function is called. The fiber will not
    //wake for any other reason
    static void yieldForever();

    //Get and set the stack size for new fibers. A value of 0 indicates that
    //the default stack size for the process will be used.
    static quint32 getDefaultStackSize();
    static void setDefaultStackSize(quint32 uiStackSize);

signals:
    //Fiber state signals
    void running();
    void waiting();
    void finished();
    void stateChanged(CoQt::Fiber::FiberState state);

public slots:
    //Causes the fiber to run. Cancels any pending wake conditions.
    void wake();
};

//This function creates a new fiber with the given function object. Usually a
//lambda, but can be a bound member function, global C function, etc...
QSharedPointer<Fiber> createFiber(const std::function<void()> &func);

//same as above, but takes an object an function pointer.
template <class T>
static QSharedPointer<Fiber> createFiber(T *object,void (T::*pFunc)())
{
    return createFiber(std::bind(pFunc, object));
}

}

Q_DECLARE_METATYPE(CoQt::Fiber::FiberState);

#endif // FIBER_H
