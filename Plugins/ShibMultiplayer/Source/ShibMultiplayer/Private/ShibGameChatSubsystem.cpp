// Fill out your copyright notice in the Description page of Project Settings.


#include "ShibGameChatSubsystem.h"

void UShibGameChatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	bGameChatUIOpen=false;
}

void UShibGameChatSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UShibGameChatSubsystem::SendMessage_Implementation(const FString& PlayerName, const FString& Message)
{
	FChatMessage ChatMessage;

	ChatMessage.Timestamp = FDateTime::UtcNow();
	ChatMessage.PlayerName = PlayerName;
	ChatMessage.Message = Message;

	// Sends a message from the player owning this component
	AddChatMessage(ChatMessage);
}


void UShibGameChatSubsystem::AddChatMessage(const FChatMessage& MessageChat)
{
	// Adds a message to a players local chat array and broadcasts it so the UI can be updated
	ChatArray.Add(MessageChat);
	ChatUpdateDelegate.Broadcast(MessageChat.PlayerName,MessageChat.Message);
}
