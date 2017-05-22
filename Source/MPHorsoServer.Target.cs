// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

[SupportedPlatforms(UnrealPlatformClass.Server)]
public class MPHorsoServerTarget : TargetRules
{
    public MPHorsoServerTarget(TargetInfo Target)
    {
        Type = TargetType.Server;
        bUsesSteam = false;
    }

    //
    // TargetRules interface.
    //

    public override void SetupBinaries(
        TargetInfo Target,
        ref List<UEBuildBinaryConfiguration> OutBuildBinaryConfigurations,
        ref List<string> OutExtraModuleNames
        )
    {
        OutExtraModuleNames.AddRange( new string[] { "MPHorso" } );
    }
}
