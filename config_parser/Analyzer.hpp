#ifndef ANALYZER_HPP
#define ANALYZER_HPP
#include <map>
#include <string>
#include <vector>

std::vector<std::string> enterBlockCtx(Directive stmt, std::vector<std::string> ctx);
bool analyze(Directive stmt, std::string term, std::vector<std::string> ctx);

int get_contexts(std::vector<std::string> blockCtx);
std::vector<int> get_directives(std::string directive);

// bit masks for different directive argument styles
const int ngxConfNoArgs = 0x00000001; // 0 args
const int ngxConfTake1  = 0x00000002; // 1 args
const int ngxConfTake2  = 0x00000004; // 2 args
const int ngxConfTake3  = 0x00000008; // 3 args
const int ngxConfTake4  = 0x00000010; // 4 args
const int ngxConfTake5  = 0x00000020; // 5 args
const int ngxConfTake6  = 0x00000040; // 6 args

// ngxConfTake7  = 0x00000080 // 7 args (currently unused)
const int ngxConfBlock  = 0x00000100; // followed by block
const int ngxConfFlag   = 0x00000200; // 'on' or 'off'
const int ngxConfAny    = 0x00000400; // >=0 args
const int ngxConf1More  = 0x00000800; // >=1 args
const int ngxConf2More  = 0x00001000; // >=2 args

// some helpful argument style aliases
const int ngxConfTake12   = (ngxConfTake1 | ngxConfTake2);
//ngxConfTake13   = (ngxConfTake1 | ngxConfTake3) (currently unused)
const int ngxConfTake23   = (ngxConfTake2 | ngxConfTake3);
const int ngxConfTake34   = (ngxConfTake3 | ngxConfTake4);
const int ngxConfTake123  = (ngxConfTake12 | ngxConfTake3);
const int ngxConfTake1234 = (ngxConfTake123 | ngxConfTake4);

// bit masks for different directive locations
const int ngxDirectConf     = 0x00010000; // main file (not used)
const int ngxMainConf       = 0x00040000; // main context
const int ngxEventConf      = 0x00080000; // events
const int ngxMailMainConf   = 0x00100000; // mail
const int ngxMailSrvConf    = 0x00200000; // mail > server
const int ngxStreamMainConf = 0x00400000; // stream
const int ngxStreamSrvConf  = 0x00800000; // stream > server
const int ngxStreamUpsConf  = 0x01000000; // stream > upstream
const int ngxHttpMainConf   = 0x02000000; // http
const int ngxHttpSrvConf    = 0x04000000; // http > server
const int ngxHttpLocConf    = 0x08000000; // http > location
const int ngxHttpUpsConf    = 0x10000000; // http > upstream
const int ngxHttpSifConf    = 0x20000000; // http > server > if
const int ngxHttpLifConf    = 0x40000000; // http > location > if
const int ngxHttpLmtConf    = 0x80000000; // http > location > limit_except

#endif
