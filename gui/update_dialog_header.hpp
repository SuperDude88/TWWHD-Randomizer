#pragma once

#ifdef QT_GUI
    #include <randomizer_thread.hpp>

    #define UPDATE_DIALOG_VALUE(val)   ((RandomizerThread*) QThread::currentThread())->dialogValueUpdate(val);
    #define UPDATE_DIALOG_LABEL(label) ((RandomizerThread*) QThread::currentThread())->dialogLabelUpdate(std::string(label).c_str());

    #define THREAD_UPDATE_DIALOG_VALUE(thread, val)   thread->dialogValueUpdate(val);
    #define THREAD_UPDATE_DIALOG_LABEL(thread, label) thread->dialogLabelUpdate(std::string(label).c_str());
#else
    #define UPDATE_DIALOG_VALUE(val) (void)(val); //avoid unused variable warning clutter
    #define UPDATE_DIALOG_LABEL(label) (void)(label); //avoid unused variable warning clutter

    #define THREAD_UPDATE_DIALOG_VALUE(thread, val)
    #define THREAD_UPDATE_DIALOG_LABEL(thread, label)
#endif
