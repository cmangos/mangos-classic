/*
 * Copyright (C) 2010-2013 Anathema Engine project <http://valkyrie-wow.com/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2016-2017 Elysium Project <https://www.elysium-project.org>
 * Copyright (C) 2017-2020 namreeb (legal@namreeb.org)
 */


#include "Auth/Sha1.h"

#ifndef _WARDEN_KEY_GENERATION_H
#define _WARDEN_KEY_GENERATION_H

class SHA1Randx
{
public:
    SHA1Randx(const uint8* buff, size_t size)
    {
        auto const taken = size/2;

        sh.Initialize();
        sh.UpdateData(buff, taken);
        sh.Finalize();

        memcpy(o1, sh.GetDigest(), 20);

        sh.Initialize();
        sh.UpdateData(buff + taken, size - taken);
        sh.Finalize();

        memcpy(o2, sh.GetDigest(), 20);

        memset(o0, 0x00, 20);

        FillUp();
    }

    void Generate(uint8* buf, size_t sz)
    {
        for (uint32 i = 0; i < sz; ++i)
        {
            if (taken == 20)
                FillUp();

            buf[i] = o0[taken];
            taken++;
        }
    }

private:
    void FillUp()
    {
        sh.Initialize();
        sh.UpdateData(o1, 20);
        sh.UpdateData(o0, 20);
        sh.UpdateData(o2, 20);
        sh.Finalize();

        memcpy(o0, sh.GetDigest(), 20);

        taken = 0;
    }

    Sha1Hash sh;
    size_t taken;
    uint8 o0[20],o1[20],o2[20];
};

#endif