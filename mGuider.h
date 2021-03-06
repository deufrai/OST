#ifndef MGUIDER_H_
#define MGUIDER_H_
#include "module.h"

class MGuider : public Module
{
    Q_OBJECT
    public:
        MGuider(MyClient *cli,Properties *properties);
        ~MGuider();

        void startCalibration(void);
        void abort(void);

        void initProperties(void);
        void executeTaskSpec(Ttask task);
        void executeTaskSpec(Ttask task,IBLOB *bp);
        void executeTaskSpec(Ttask task,INumberVectorProperty *nvp) ;
        void executeTaskSpec(Ttask task,ISwitchVectorProperty *svp) ;
        void executeTaskSpec(Ttask task,ITextVectorProperty *tvp) ;
        void executeTaskSpec(Ttask task,ILightVectorProperty *lvp) ;
        void slotvalueChangedFromCtl(Prop prop);

   public slots:
        void test(void);

};
#endif
