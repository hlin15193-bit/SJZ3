#include "hook22.h"
#include "main.h"


Int3Hook Hook;
#define DegreesToRadians(deg) ((deg) * (3.14159265358979323846f / 180.0f))


float Fmod(float X, float Y) {
    return fmodf(X, Y);

}

void SinCos(float* OutSin, float* OutCos, float Radians) {
    *OutSin = sinf(Radians);
    *OutCos = cosf(Radians);
}

int ContainsNaN(SDK::FVector V) {
    return isnan(V.X) || isnan(V.Y) || isnan(V.Z);
}

SDK::FVector GetForwardVector(const SDK::FRotator* InRo)
{
    const float PitchNoWinding = Fmod(InRo->Pitch, 360.0f);
    const float YawNoWinding = Fmod(InRo->Yaw, 360.0f);
    float SP, CP, SY, CY;
    SinCos(&SP, &CP, DegreesToRadians(PitchNoWinding));
    SinCos(&SY, &CY, DegreesToRadians(YawNoWinding));
    SDK::FVector V = { CP * CY,CP * SY,SP };
    if (ContainsNaN(V))
    {
        return SDK::FVector();
    }

    return V;
}


LONG NTAPI AimbotException(_EXCEPTION_POINTERS* ExceptionInfo)
{
    if (ExceptionInfo->ExceptionRecord->ExceptionAddress == Hook.GetHookAddr())
    {
        Hook.UnHook();
        Hook.HookJmpAddr();
        SDK::AIntCharacter* Instance = _player;
        SDK::FVector* SpawnLocation = (SDK::FVector*)(ExceptionInfo->ContextRecord->Rcx);
        if (Isvalidptr(SpawnLocation) && Isvalidptr(Instance))
        {
            if (×·×Ù¿ª¹Ø && GetAsyncKeyState(GetKeyCodeFromSelection(selected_key)))
            {
                auto aimpos = GetBonePos(Instance->Mesh, GetBoneNameW(selected_part));
                SDK::FRotator AimRota = GetAimRotation(aimpos);
                SDK::FVector AimVector = GetForwardVector(&AimRota);
                
                SpawnLocation->X = AimVector.X;
                SpawnLocation->Y = AimVector.Y;
                SpawnLocation->Z = AimVector.Z;

            }
            
        }
       
        return -1;
    }
    else if (ExceptionInfo->ExceptionRecord->ExceptionAddress == Hook.GetJmpAddr())
    {
        Hook.Hook();
        Hook.UnHookJmpAddr();
        return -1;
    }
    return 0;
}

void HookInitialize()
{
    ULONG64 SpawAddress = SDK::InSDKUtils::GetImageBase() + 0x3B34A4D;
    Hook.Initialize((LPVOID)(SpawAddress), 5, AimbotException);
    Hook.Hook();
}
