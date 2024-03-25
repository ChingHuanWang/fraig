#include "fec.h"
#include "cirGate.h"

FecGrp::FecGrp(const vector<CirGate*>& dfsList) 
{
    for (size_t i = 0; i < dfsList.size(); i++)
        if (dfsList[i]->isAig()) 
            _vec.push_back(dfsList[i]->getId()*2); 
}

ostream&
operator << (ostream& os, FecGrp& grp)
{
    for (size_t i = 0; i < grp.size(); i++)
        if (grp[i]%2)
            cout << '!' << grp[i]/2 << " ";
        else
            cout << grp[i]/2 << " ";
    cout << endl;
    return os;
}