#include "Misc/AutomationTest.h"
#include "AnimDataFromJSON.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAddTest, "GameTests.Math.Add", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool FAddTest::RunTest(const FString& Parameters)
{

    TestEqual(TEXT("Add 2 and 3"),2+3, 5);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSubtractTest, "GameTests.Math.Subtract", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool FSubtractTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("Subtract 5 from 10"),10-5, 5);

    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExtractNumberTest, "GameTests.Math.ExtractNumber", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool FExtractNumberTest::RunTest(const FString& Parameters)
{
    FString TestFilename = "frame_123.json";
    int32 Expected = 123;
    FAnimDataFromJSONModule AnimDataModule; 
    int32 Result = AnimDataModule.ExtractNumberFromFilename(TestFilename);

    TestEqual(TEXT("The number should be 123"), Result, Expected);

    return true;

}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFileTest, "GameTests.Math.FileOrder", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool FFileTest::RunTest(const FString& Parameters)
{
    FAnimDataFromJSONModule AnimDataModule;
    FString TestFilename1 = "frame_2.json";
    FString TestFilename2 = "frame_1.json";
    int32 Result1 = AnimDataModule.ExtractNumberFromFilename(TestFilename1);
    int32 Result2 = AnimDataModule.ExtractNumberFromFilename(TestFilename2);
    TArray<int32> TestFilename;
    TestFilename.Add(Result1);
    TestFilename.Add(Result2);
    TestFilename.Sort([this](const int32& A, const int32& B) {
        return A < B;
        });
    TArray<int32> Expected = { 1,2 };
    TestEqual(TEXT("The order should be "), TestFilename, Expected);
    return true;

}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJSONTest, "GameTests.Math.JSON", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool FJSONTest::RunTest(const FString& Parameters)
{
    FString JsonString = TEXT("{\
        \"default:camera1\": {\
            \"tx\": 2,\
            \"ty\" : 3.7265,\
            \"tz\" : 9.365426667646583,\
            \"rx\" : 0.0,\
            \"ry\" : 26.28676872443435,\
            \"rz\" : 0.0,\
        },\
        \"default:camera2\": {\
            \"tx\": 0.0,\
            \"ty\" : 4.141543443289037,\
            \"tz\" : 0.0,\
            \"rx\" : 0.0,\
            \"ry\" : 0.0,\
            \"rz\" : 0.0,\
        }\
    }");
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {

        for (const auto& Pair : JsonObject->Values)
        {
            FName camName = FName(*Pair.Key);
            const TSharedPtr<FJsonObject> camDataObject = Pair.Value->AsObject();
            if (camDataObject.IsValid())
            {
                // Check tx value for "default:camera1"
                if (camName.ToString() == "default:camera1")
                {
                    int32 tx = camDataObject->GetNumberField("tx");
                    TestEqual(TEXT("Camera1 TX should match."), tx, 2);
                    return true;
                }
            }

        }
    }
    return false;
}






