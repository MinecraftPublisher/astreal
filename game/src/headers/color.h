#pragma once

// Regular text
#define BLK "\1\e[0;30m\2"
#define RED "\1\e[0;31m\2"
#define GRN "\1\e[0;32m\2"
#define YEL "\1\e[0;33m\2"
#define BLU "\1\e[0;34m\2"
#define MAG "\1\e[0;35m\2"
#define CYN "\1\e[0;36m\2"
#define WHT "\1\e[0;37m\2"

// Regular bold text
#define BBLK "\1\e[1;30m\2"
#define BRED "\1\e[1;31m\2"
#define BGRN "\1\e[1;32m\2"
#define BYEL "\1\e[1;33m\2"
#define BBLU "\1\e[1;34m\2"
#define BMAG "\1\e[1;35m\2"
#define BCYN "\1\e[1;36m\2"
#define BWHT "\1\e[1;37m\2"

// Regular underline text
#define UBLK "\1\e[4;30m\2"
#define URED "\1\e[4;31m\2"
#define UGRN "\1\e[4;32m\2"
#define UYEL "\1\e[4;33m\2"
#define UBLU "\1\e[4;34m\2"
#define UMAG "\1\e[4;35m\2"
#define UCYN "\1\e[4;36m\2"
#define UWHT "\1\e[4;37m\2"

// Regular background
#define BLKB "\1\e[40m\2"
#define REDB "\1\e[41m\2"
#define GRNB "\1\e[42m\2"
#define YELB "\1\e[43m\2"
#define BLUB "\1\e[44m\2"
#define MAGB "\1\e[45m\2"
#define CYNB "\1\e[46m\2"
#define WHTB "\1\e[47m\2"

// High intensty background
#define BLKHB "\1\e[0;100m\2"
#define REDHB "\1\e[0;101m\2"
#define GRNHB "\1\e[0;102m\2"
#define YELHB "\1\e[0;103m\2"
#define BLUHB "\1\e[0;104m\2"
#define MAGHB "\1\e[0;105m\2"
#define CYNHB "\1\e[0;106m\2"
#define WHTHB "\1\e[0;107m\2"

// High intensty text
#define HBLK "\1\e[0;90m\2"
#define HRED "\1\e[0;91m\2"
#define HGRN "\1\e[0;92m\2"
#define HYEL "\1\e[0;93m\2"
#define HBLU "\1\e[0;94m\2"
#define HMAG "\1\e[0;95m\2"
#define HCYN "\1\e[0;96m\2"
#define HWHT "\1\e[0;97m\2"

// Bold high intensity text
#define BHBLK "\1\e[1;90m\2"
#define BHRED "\1\e[1;91m\2"
#define BHGRN "\1\e[1;92m\2"
#define BHYEL "\1\e[1;93m\2"
#define BHBLU "\1\e[1;94m\2"
#define BHMAG "\1\e[1;95m\2"
#define BHCYN "\1\e[1;96m\2"
#define BHWHT "\1\e[1;97m\2"

// Reset
#define reset       "\1\e[0m\2"
#define RESET       "\1\e[0m\2"
#define CRESET      "\1\e[0m\2"
#define COLOR_RESET "\1\e[0m\2"

// Regular text
#define cBLK(x) "\1\e[0;30m\2" x RESET
#define cRED(x) "\1\e[0;31m\2" x RESET
#define cGRN(x) "\1\e[0;32m\2" x RESET
#define cYEL(x) "\1\e[0;33m\2" x RESET
#define cBLU(x) "\1\e[0;34m\2" x RESET
#define cMAG(x) "\1\e[0;35m\2" x RESET
#define cCYN(x) "\1\e[0;36m\2" x RESET
#define cWHT(x) "\1\e[0;37m\2" x RESET

#define cBLK(x) "\1\e[0;30m\2" x RESET
#define cRED(x) "\1\e[0;31m\2" x RESET
#define cGRN(x) "\1\e[0;32m\2" x RESET
#define cYEL(x) "\1\e[0;33m\2" x RESET
#define cBLU(x) "\1\e[0;34m\2" x RESET
#define cMAG(x) "\1\e[0;35m\2" x RESET
#define cCYN(x) "\1\e[0;36m\2" x RESET
#define cWHT(x) "\1\e[0;37m\2" x RESET

// Regular bold text
#define cBBLK(x) "\1\e[1;30m\2" x RESET
#define cBRED(x) "\1\e[1;31m\2" x RESET
#define cBGRN(x) "\1\e[1;32m\2" x RESET
#define cBYEL(x) "\1\e[1;33m\2" x RESET
#define cBBLU(x) "\1\e[1;34m\2" x RESET
#define cBMAG(x) "\1\e[1;35m\2" x RESET
#define cBCYN(x) "\1\e[1;36m\2" x RESET
#define cBWHT(x) "\1\e[1;37m\2" x RESET

// Regular underline text
#define cUBLK(x) "\1\e[4;30m\2" x RESET
#define cURED(x) "\1\e[4;31m\2" x RESET
#define cUGRN(x) "\1\e[4;32m\2" x RESET
#define cUYEL(x) "\1\e[4;33m\2" x RESET
#define cUBLU(x) "\1\e[4;34m\2" x RESET
#define cUMAG(x) "\1\e[4;35m\2" x RESET
#define cUCYN(x) "\1\e[4;36m\2" x RESET
#define cUWHT(x) "\1\e[4;37m\2" x RESET

// Regular background
#define cBLKB(x) "\1\e[40m\2" x RESET
#define cREDB(x) "\1\e[41m\2" x RESET
#define cGRNB(x) "\1\e[42m\2" x RESET
#define cYELB(x) "\1\e[43m\2" x RESET
#define cBLUB(x) "\1\e[44m\2" x RESET
#define cMAGB(x) "\1\e[45m\2" x RESET
#define cCYNB(x) "\1\e[46m\2" x RESET
#define cWHTB(x) "\1\e[47m\2" x RESET

// High intensty background
#define cBLKHB(x) "\1\e[0;100m\2" x RESET
#define cREDHB(x) "\1\e[0;101m\2" x RESET
#define cGRNHB(x) "\1\e[0;102m\2" x RESET
#define cYELHB(x) "\1\e[0;103m\2" x RESET
#define cBLUHB(x) "\1\e[0;104m\2" x RESET
#define cMAGHB(x) "\1\e[0;105m\2" x RESET
#define cCYNHB(x) "\1\e[0;106m\2" x RESET
#define cWHTHB(x) "\1\e[0;107m\2" x RESET

// High intensty text
#define cHBLK(x) "\1\e[0;90m\2" x RESET
#define cHRED(x) "\1\e[0;91m\2" x RESET
#define cHGRN(x) "\1\e[0;92m\2" x RESET
#define cHYEL(x) "\1\e[0;93m\2" x RESET
#define cHBLU(x) "\1\e[0;94m\2" x RESET
#define cHMAG(x) "\1\e[0;95m\2" x RESET
#define cHCYN(x) "\1\e[0;96m\2" x RESET
#define cHWHT(x) "\1\e[0;97m\2" x RESET

// Bold high intensity text
#define cBHBLK(x) "\1\e[1;90m\2" x RESET
#define cBHRED(x) "\1\e[1;91m\2" x RESET
#define cBHGRN(x) "\1\e[1;92m\2" x RESET
#define cBHYEL(x) "\1\e[1;93m\2" x RESET
#define cBHBLU(x) "\1\e[1;94m\2" x RESET
#define cBHMAG(x) "\1\e[1;95m\2" x RESET
#define cBHCYN(x) "\1\e[1;96m\2" x RESET
#define cBHWHT(x) "\1\e[1;97m\2" x RESET