#pragma once

namespace ai
{
    class LogAnalysis
    {
    public:
        //Constructors
        LogAnalysis() {};

        static void RunAnalysis();

        static void AnalysePid();
        static void AnalyseEvents();
        static void AnalyseQuests();
        static void AnalyseCounts();
    };    
}