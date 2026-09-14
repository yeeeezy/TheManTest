#include "Enemy/Boss/CoreMorph/AI/BTDecorator_CoreMorphBool.h"
void UBTDecorator_CoreMorphBool::Configure(FName Key,EBTFlowAbortMode::Type AbortMode)
{
    BlackboardKey.SelectedKeyName=Key;BlackboardKey.AddBoolFilter(this,GET_MEMBER_NAME_CHECKED(UBTDecorator_CoreMorphBool,BlackboardKey));
    FlowAbortMode=AbortMode;OperationType=uint8(EBasicKeyOperation::Set);
#if WITH_EDITORONLY_DATA
    BasicOperation=EBasicKeyOperation::Set;
#endif
    NodeName=Key.ToString();
}
