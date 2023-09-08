#pragma once

#ifdef QT_GUI
    #include <randomizer_thread.hpp>

    #define UPDATE_DIALOG_VALUE(val)   TheMainThread::mainThread->dialogValueUpdate(val);
    #define UPDATE_DIALOG_LABEL(label) TheMainThread::mainThread->dialogLabelUpdate(std::string(label).c_str());
    #define UPDATE_DIALOG_TITLE(title) TheMainThread::mainThread->dialogTitleUpdate(std::string(title).c_str());

#else
    #define UPDATE_DIALOG_VALUE(val) (void)(val); //avoid unused variable warning clutter
    #define UPDATE_DIALOG_LABEL(label) (void)(label); //avoid unused variable warning clutter
    #define UPDATE_DIALOG_TITLE(title) (void)(title); //avoid unused variable warning clutter

#endif
