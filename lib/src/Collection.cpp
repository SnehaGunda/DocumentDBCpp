/***
* The MIT License (MIT)
*
* Copyright (c) 2015 DocumentDBCpp
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
***/

#pragma comment(lib, "rpcrt4.lib")  // UuidCreate - Minimum supported OS Win 2000

#include "Collection.h"

#include <windows.h>

#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/json.h>

#include "ConnectionHelper.h"
#include "DocumentDBConstants.h"
#include "exceptions.h"

using namespace documentdb;
using namespace std;
using namespace web::http;
using namespace web::json;
using namespace web::http::client;

Collection::Collection(
		const shared_ptr<const DocumentDBConfiguration>& document_db_configuration,
		const wstring& id,
		const wstring& resource_id,
		const unsigned long ts,
		const wstring& self,
		const wstring& etag,
		const wstring& docs,
		const wstring& sprocs,
		const wstring& triggers,
		const wstring& udfs,
		const wstring& conflicts,
		const IndexingPolicy& indexing_policy)
	: DocumentDBEntity(document_db_configuration, id, resource_id, ts, self, etag)
	, docs_(docs)
	, sprocs_(sprocs)
	, triggers_(triggers)
	, udfs_(udfs)
	, conflicts_(conflicts)
	, indexing_policy_(indexing_policy)
{}

Collection::~Collection()
{}

shared_ptr<Document> Collection::DocumentFromJson(
	const value& json_collection) const
{
	wstring id = json_collection.at(DOCUMENT_ID).as_string();
	wstring rid = json_collection.at(RESPONSE_RESOURCE_RID).as_string();
	unsigned long ts = json_collection.at(RESPONSE_RESOURCE_TS).as_integer();
	wstring self = json_collection.at(RESPONSE_RESOURCE_SELF).as_string();
	wstring etag = json_collection.at(RESPONSE_RESOURCE_ETAG).as_string();
	wstring attachments = json_collection.at(RESPONSE_RESOURCE_ATTACHMENTS).as_string();

	return make_shared<Document>(
		this->document_db_configuration(),
		id,
		rid,
		ts,
		self,
		etag,
		attachments,
		json_collection);
}

shared_ptr<Trigger> Collection::TriggerFromJson(
	const value* json_trigger) const
{
	wstring id = json_trigger->at(DOCUMENT_ID).as_string();
	wstring rid = json_trigger->at(RESPONSE_RESOURCE_RID).as_string();
	unsigned long ts = json_trigger->at(RESPONSE_RESOURCE_TS).as_integer();
	wstring self = json_trigger->at(RESPONSE_RESOURCE_SELF).as_string();
	wstring etag = json_trigger->at(RESPONSE_RESOURCE_ETAG).as_string();
	wstring body = json_trigger->at(RESPONSE_RESOURCE_BODY).as_string();
	wstring trigger_operation_str = json_trigger->at(RESPONSE_RESOURCE_TRIGGER_OPERATION).as_string();
	TriggerOperation triggerOperation = TriggerOperation::ALL;
	if (compare(trigger_operation_str, L"ALL"))
	{
	}
	else if (compare(trigger_operation_str, L"UPDATE"))
	{
		triggerOperation = TriggerOperation::UPDATE;
	}
	else if (compare(trigger_operation_str, L"CREATE"))
	{
		triggerOperation = TriggerOperation::CREATE;
	}
	else if (compare(trigger_operation_str, L"REPLACE"))
	{
		triggerOperation = TriggerOperation::REPLACE;
	}
	else if (compare(trigger_operation_str, L"DELETE"))
	{
		triggerOperation = TriggerOperation::DEL;
	}
	else
	{
		throw DocumentDBRuntimeException(L"Unsupported trigger operation.");
	}
	wstring trigger_type_str = json_trigger->at(RESPONSE_RESOURCE_TRIGGER_TYPE).as_string();
	TriggerType triggerType = TriggerType::PRE;
	if (compare(trigger_type_str, L"PRE"))
	{
	}
	else if (compare(trigger_type_str, L"POST"))
	{
		triggerType = TriggerType::POST;
	}
	else
	{
			throw DocumentDBRuntimeException(L"Unsupported trigger type.");
	}

	IndexingPolicy indexing_policy;
	if (json_trigger->has_field(RESPONSE_INDEXING_POLICY))
	{
		value indexing_policy_json = json_trigger->at(RESPONSE_INDEXING_POLICY);
		indexing_policy = IndexingPolicy::FromJson(indexing_policy_json);
	}

	return make_shared<Trigger>(
		this->document_db_configuration(),
		id,
		rid,
		ts,
		self,
		etag,
		body,
		triggerOperation,
		triggerType);
}

shared_ptr<StoredProcedure> Collection::StoredProcedureFromJson(
	const value* json_sproc) const
{
	wstring id = json_sproc->at(DOCUMENT_ID).as_string();
	wstring rid = json_sproc->at(RESPONSE_RESOURCE_RID).as_string();
	unsigned long ts = json_sproc->at(RESPONSE_RESOURCE_TS).as_integer();
	wstring self = json_sproc->at(RESPONSE_RESOURCE_SELF).as_string();
	wstring etag = json_sproc->at(RESPONSE_RESOURCE_ETAG).as_string();
	wstring body = json_sproc->at(RESPONSE_RESOURCE_BODY).as_string();

	IndexingPolicy indexing_policy;
	if (json_sproc->has_field(RESPONSE_INDEXING_POLICY))
	{
		value indexing_policy_json = json_sproc->at(RESPONSE_INDEXING_POLICY);
		indexing_policy = IndexingPolicy::FromJson(indexing_policy_json);
	}

	return make_shared<StoredProcedure>(
		this->document_db_configuration(),
		id,
		rid,
		ts,
		self,
		etag,
		body);
}

std::shared_ptr<UserDefinedFunction> Collection::UserDefinedFunctionFromJson(
	const value* json_udf) const
{
	wstring id = json_udf->at(DOCUMENT_ID).as_string();
	wstring rid = json_udf->at(RESPONSE_RESOURCE_RID).as_string();
	unsigned long ts = json_udf->at(RESPONSE_RESOURCE_TS).as_integer();
	wstring self = json_udf->at(RESPONSE_RESOURCE_SELF).as_string();
	wstring etag = json_udf->at(RESPONSE_RESOURCE_ETAG).as_string();
	wstring body = json_udf->at(RESPONSE_RESOURCE_BODY).as_string();

	IndexingPolicy indexing_policy;
	if (json_udf->has_field(RESPONSE_INDEXING_POLICY))
	{
		value indexing_policy_json = json_udf->at(RESPONSE_INDEXING_POLICY);
		indexing_policy = IndexingPolicy::FromJson(indexing_policy_json);
	}

	return make_shared<UserDefinedFunction>(
		this->document_db_configuration(),
		id,
		rid,
		ts,
		self,
		etag,
		body);
}

wstring Collection::GenerateGuid()
{
	UUID uuid;
	ZeroMemory(&uuid, sizeof(UUID));
	RPC_STATUS status = UuidCreate(&uuid);
	if (status != RPC_S_OK)
	{
		throw DocumentDBRuntimeException(L"Unable to create UUID");
	}

	wchar_t *str;
	status = UuidToStringW(&uuid, (RPC_WSTR*)&str);
	if (status == RPC_S_OUT_OF_MEMORY)
	{
		throw DocumentDBRuntimeException(L"Out of memory while creating UUID");
	}
	else if (status != RPC_S_OK)
	{
		throw DocumentDBRuntimeException(L"Unknown error while creating UUID");
	}

	wstring guid = str;
	RpcStringFreeW((RPC_WSTR*)&str);

	return guid;
}

Concurrency::task<shared_ptr<Document>> Collection::CreateDocumentAsync(
	const value& document) const
{
	value body = document;

	if (!body.has_field(DOCUMENT_ID))
	{
		body[DOCUMENT_ID] = value::string(GenerateGuid());
	}

	return this->CreateDocumentAsync(body.serialize());
}

Concurrency::task<shared_ptr<Document>> Collection::CreateDocumentAsync(
	const wstring& document) const
{
	http_request request = CreateRequest(
		methods::POST,
		RESOURCE_PATH_DOCS,
		this->resource_id(),
		this->document_db_configuration()->master_key());
	request.set_request_uri(this->self() + docs_);

	request.set_body(document);

	return this->document_db_configuration()->http_client().request(request).then([=](http_response response)
	{
		value json_response = response.extract_json().get();

		if (response.status_code() == status_codes::Created)
		{
			return DocumentFromJson(json_response);
		}

		ThrowExceptionFromResponse(response.status_code(), json_response);
	});
}

shared_ptr<Document> Collection::CreateDocument(
	const value& document) const
{
	return this->CreateDocumentAsync(document).get();
}

shared_ptr<Document> Collection::CreateDocument(
	const wstring& document) const
{
	return this->CreateDocumentAsync(document).get();
}

Concurrency::task<shared_ptr<Document>> Collection::GetDocumentAsync(
	const wstring& resource_id) const
{
	http_request request = CreateRequest(
		methods::GET,
		RESOURCE_PATH_DOCS,
		resource_id,
		this->document_db_configuration()->master_key());
	request.set_request_uri(this->self() + docs_ + resource_id);

	return this->document_db_configuration()->http_client().request(request).then([=](http_response response)
	{
		value json_response = response.extract_json().get();

		if (response.status_code() == status_codes::OK)
		{
			return DocumentFromJson(json_response);
		}

		ThrowExceptionFromResponse(response.status_code(), json_response);
	});
}

shared_ptr<Document> Collection::GetDocument(
	const wstring& resource_id) const
{
	return this->GetDocumentAsync(resource_id).get();
}

Concurrency::task<vector<shared_ptr<Document>>> Collection::ListDocumentsAsync() const
{
	http_request request = CreateRequest(
		methods::GET,
		RESOURCE_PATH_DOCS,
		this->resource_id(),
		this->document_db_configuration()->master_key());
	request.set_request_uri(this->self() + docs_);
	return this->document_db_configuration()->http_client().request(request).then([=](http_response response)
	{
		value json_response = response.extract_json().get();

		if (response.status_code() == status_codes::OK)
		{
			assert(this->resource_id() == json_response.at(RESPONSE_RESOURCE_RID).as_string());
			vector<shared_ptr<Document>> documents;
			documents.reserve(json_response.at(RESPONSE_BODY_COUNT).as_integer());
			value json_documents = json_response.at(RESPONSE_QUERY_DOCUMENTS);

			for (auto iter = json_documents.as_array().cbegin(); iter != json_documents.as_array().cend(); ++iter)
			{
				shared_ptr<Document> coll = DocumentFromJson(*iter);
				documents.push_back(coll);
			}
			return documents;
		}

		ThrowExceptionFromResponse(response.status_code(), json_response);
	});
}

vector<shared_ptr<Document>> Collection::ListDocuments() const
{
	return this->ListDocumentsAsync().get();
}

Concurrency::task<shared_ptr<Document>> Collection::ReplaceDocumentAsync(
	const wstring& resource_id,
	const value& document) const
{
	http_request request = CreateRequest(
		methods::PUT,
		RESOURCE_PATH_DOCS,
		resource_id,
		this->document_db_configuration()->master_key());
	request.set_request_uri(this->self() + docs_ + resource_id);

	value body = document;
	if (!body.has_field(DOCUMENT_ID))
	{
		body[DOCUMENT_ID] = value::string(GenerateGuid());
	}

	request.set_body(body);

	return this->document_db_configuration()->http_client().request(request).then([=](http_response response)
	{
		value json_response = response.extract_json().get();

		if (response.status_code() == status_codes::OK)
		{
			assert(resource_id == json_response.at(RESPONSE_RESOURCE_RID).as_string());
			return DocumentFromJson(json_response);
		}

		ThrowExceptionFromResponse(response.status_code(), json_response);
	});
}

shared_ptr<Document> Collection::ReplaceDocument(
	const wstring& resource_id,
	const value& document) const
{
	return this->ReplaceDocumentAsync(resource_id, document).get();
}

Concurrency::task<void> Collection::DeleteDocumentAsync(
	const shared_ptr<Document>& document) const
{
	return DeleteDocumentAsync(document->resource_id());
}

void Collection::DeleteDocument(
	const shared_ptr<Document>& document) const
{
	this->DeleteDocumentAsync(document).get();
}

Concurrency::task<void> Collection::DeleteDocumentAsync(
	const wstring& resource_id) const
{
	http_request request = CreateRequest(
		methods::DEL,
		RESOURCE_PATH_DOCS,
		resource_id,
		this->document_db_configuration()->master_key());
	request.set_request_uri(this->self() + docs_ + resource_id);

	return this->document_db_configuration()->http_client().request(request).then([=](http_response response)
	{
		if (response.status_code() == status_codes::NoContent)
		{
			return;
		}

		value json_response = response.extract_json().get();
		ThrowExceptionFromResponse(response.status_code(), json_response);
	});
}

void Collection::DeleteDocument(
	const wstring& resource_id) const
{
	this->DeleteDocumentAsync(resource_id).get();
}

Concurrency::task<shared_ptr<DocumentIterator>> Collection::QueryDocumentsAsync(
	const wstring& query,
	const int page_size) const
{
	http_request request = CreateQueryRequest(
		query,
		page_size,
		RESOURCE_PATH_DOCS,
		this->resource_id(),
		this->document_db_configuration()->master_key());
	const wstring requestUri = this->self() + docs_;
	request.set_request_uri(requestUri);

	return this->document_db_configuration()->http_client().request(request).then([=](http_response response)
	{
		wstring continuation_id = response.headers()[HEADER_MS_CONTINUATION];
		value json_response = response.extract_json().get();

		if (response.status_code() == status_codes::OK)
		{
			assert(this->resource_id() == json_response.at(RESPONSE_RESOURCE_RID).as_string());

			return make_shared<DocumentIterator>(
				shared_from_this(),
				query,
				page_size,
				requestUri,
				continuation_id,
				json_response.at(RESPONSE_QUERY_DOCUMENTS));
		}

		ThrowExceptionFromResponse(response.status_code(), json_response);
	});
}

shared_ptr<DocumentIterator> Collection::QueryDocuments(
	const wstring& query,
	const int page_size) const
{
	return this->QueryDocumentsAsync(query, page_size).get();
}

Concurrency::task<shared_ptr<Trigger>> Collection::CreateTriggerAsync(
	const wstring& id,
	const wstring& body,
	const TriggerOperation& triggerOperation,
	const TriggerType& triggerType) const
{
	http_request request = CreateRequest(
		methods::POST,
		RESOURCE_PATH_TRIGGERS,
		this->resource_id(),
		this->document_db_configuration()->master_key());
	request.set_request_uri(this->self() + triggers_);
	
	value body_;
	body_[DOCUMENT_ID] = value::string(id);
	body_[BODY] = value::string(body);
	switch (triggerOperation) {
	case TriggerOperation::ALL:
		body_[TRIGGER_OPERATION] = value::string(L"All");
		break;
	case TriggerOperation::CREATE:
		body_[TRIGGER_OPERATION] = value::string(L"Create");
		break;
	case TriggerOperation::UPDATE:
		body_[TRIGGER_OPERATION] = value::string(L"Update");
		break;
	case TriggerOperation::REPLACE:
		body_[TRIGGER_OPERATION] = value::string(L"Replace");
		break;
	case TriggerOperation::DEL:
		body_[TRIGGER_OPERATION] = value::string(L"Delete");
		break;
	}
	switch (triggerType) {
	case TriggerType::PRE:
		body_[TRIGGER_TYPE] = value::string(L"Pre");
		break;
	case TriggerType::POST:
		body_[TRIGGER_TYPE] = value::string(L"Post");
		break;
	}
	
	request.set_body(body_);

	return this->document_db_configuration()->http_client().request(request).then([=](http_response response)
	{
		value json_response = response.extract_json().get();

		if (response.status_code() == status_codes::Created)
		{
			return TriggerFromJson(&json_response);
		}

		ThrowExceptionFromResponse(response.status_code(), json_response);
	});
}

shared_ptr<Trigger> Collection::CreateTrigger(
	const wstring& id,
	const wstring& body,
	const TriggerOperation& triggerOperation,
	const TriggerType& triggerType) const
{
	return CreateTriggerAsync(id, body, triggerOperation, triggerType).get();
}

Concurrency::task<shared_ptr<Trigger>> Collection::GetTriggerAsync(
	const wstring& resource_id) const
{
	http_request request = CreateRequest(
		methods::GET,
		RESOURCE_PATH_TRIGGERS,
		resource_id,
		this->document_db_configuration()->master_key());
	request.set_request_uri(this->self() + triggers_ + resource_id);

	return this->document_db_configuration()->http_client().request(request).then([=](http_response response)
	{
		value json_response = response.extract_json().get();

		if (response.status_code() == status_codes::OK)
		{
			return TriggerFromJson(&json_response);
		}

		ThrowExceptionFromResponse(response.status_code(), json_response);
	});
}

shared_ptr<Trigger> Collection::GetTrigger(
	const wstring& resource_id) const
{
	return GetTriggerAsync(resource_id).get();
}

Concurrency::task<vector<shared_ptr<Trigger>>> Collection::ListTriggersAsync() const
{
	http_request request = CreateRequest(
		methods::GET,
		RESOURCE_PATH_TRIGGERS,
		this->resource_id(),
		this->document_db_configuration()->master_key());
	request.set_request_uri(this->self() + triggers_);
	return this->document_db_configuration()->http_client().request(request).then([=](http_response response)
	{
		value json_response = response.extract_json().get();

		if (response.status_code() == status_codes::OK)
		{
			assert(this->resource_id() == json_response.at(RESPONSE_RESOURCE_RID).as_string());
			vector<shared_ptr<Trigger>> triggers;
			triggers.reserve(json_response.at(RESPONSE_BODY_COUNT).as_integer());
			value json_documents = json_response.at(RESPONSE_QUERY_TRIGGERS);

			for (auto iter = json_documents.as_array().cbegin(); iter != json_documents.as_array().cend(); ++iter)
			{
				shared_ptr<Trigger> coll = TriggerFromJson(&(*iter));
				triggers.push_back(coll);
			}
			return triggers;
		}

		ThrowExceptionFromResponse(response.status_code(), json_response);
	});
}

vector<shared_ptr<Trigger>> Collection::ListTriggers() const
{
	return ListTriggersAsync().get();
}

Concurrency::task<shared_ptr<Trigger>> Collection::ReplaceTriggerAsync(
	const wstring& id,
	const wstring& new_id,
	const wstring& body,
	const TriggerOperation& triggerOperation,
	const TriggerType& triggerType) const
{
	http_request request = CreateRequest(
		methods::PUT,
		RESOURCE_PATH_TRIGGERS,
		id,
		this->document_db_configuration()->master_key());
	request.set_request_uri(this->self() + triggers_ + id);

	value body_;
	body_[DOCUMENT_ID] = value::string(new_id);
	body_[BODY] = value::string(body);
	switch (triggerOperation) {
	case TriggerOperation::ALL:
		body_[TRIGGER_OPERATION] = value::string(L"All");
		break;
	case TriggerOperation::CREATE:
		body_[TRIGGER_OPERATION] = value::string(L"Create");
		break;
	case TriggerOperation::UPDATE:
		body_[TRIGGER_OPERATION] = value::string(L"Update");
		break;
	case TriggerOperation::REPLACE:
		body_[TRIGGER_OPERATION] = value::string(L"Replace");
		break;
	case TriggerOperation::DEL:
		body_[TRIGGER_OPERATION] = value::string(L"Delete");
		break;
	}
	switch (triggerType) {
	case TriggerType::PRE:
		body_[TRIGGER_TYPE] = value::string(L"Pre");
		break;
	case TriggerType::POST:
		body_[TRIGGER_TYPE] = value::string(L"Post");
		break;
	}

	request.set_body(body_);

	return this->document_db_configuration()->http_client().request(request).then([=](http_response response)
	{
		value json_response = response.extract_json().get();

		if (response.status_code() == status_codes::OK)
		{
			return TriggerFromJson(&json_response);
		}

		ThrowExceptionFromResponse(response.status_code(), json_response);
	});
}

shared_ptr<Trigger> Collection::ReplaceTrigger(
	const wstring& id,
	const wstring& new_id,
	const wstring& body,
	const TriggerOperation& triggerOperation,
	const TriggerType& triggerType) const
{
	return this->ReplaceTriggerAsync(id, new_id, body, triggerOperation, triggerType).get();
}

Concurrency::task<void> Collection::DeleteTriggerAsync(
	const shared_ptr<Trigger>& trigger) const
{
	return DeleteTriggerAsync(trigger->resource_id());
}

void Collection::DeleteTrigger(
	const shared_ptr<Trigger>& trigger) const
{
	DeleteTriggerAsync(trigger->resource_id()).get();
}

Concurrency::task<void> Collection::DeleteTriggerAsync(
	const wstring& resource_id) const
{
	http_request request = CreateRequest(
		methods::DEL,
		RESOURCE_PATH_TRIGGERS,
		resource_id,
		this->document_db_configuration()->master_key());
	request.set_request_uri(this->self() + triggers_ + resource_id);

	return this->document_db_configuration()->http_client().request(request).then([=](http_response response)
	{
		if (response.status_code() == status_codes::NoContent)
		{
			return;
		}

		value json_response = response.extract_json().get();
		ThrowExceptionFromResponse(response.status_code(), json_response);
	});
}

void Collection::DeleteTrigger(
	const wstring& resource_id) const
{
	DeleteTriggerAsync(resource_id).get();
}

Concurrency::task<shared_ptr<TriggerIterator>> Collection::QueryTriggersAsync(
	const wstring& query,
	const int page_size) const
{
	http_request request = CreateQueryRequest(
		query,
		page_size,
		RESOURCE_PATH_TRIGGERS,
		this->resource_id(),
		this->document_db_configuration()->master_key());
	const wstring requestUri = this->self() + triggers_;
	request.set_request_uri(requestUri);

	return this->document_db_configuration()->http_client().request(request).then([=](http_response response)
	{
		wstring continuation_id = response.headers()[HEADER_MS_CONTINUATION];
		value json_response = response.extract_json().get();

		if (response.status_code() == status_codes::OK)
		{
			assert(this->resource_id() == json_response.at(RESPONSE_RESOURCE_RID).as_string());

			return make_shared<TriggerIterator>(
				shared_from_this(),
				query,
				page_size,
				requestUri,
				continuation_id,
				json_response.at(RESPONSE_QUERY_TRIGGERS));
		}

		ThrowExceptionFromResponse(response.status_code(), json_response);
	});
}

shared_ptr<TriggerIterator> Collection::QueryTriggers(
	const wstring& query,
	const int page_size) const
{
	return QueryTriggersAsync(query, page_size).get();
}

Concurrency::task<shared_ptr<StoredProcedure>> Collection::CreateStoredProcedureAsync(
	const wstring& id,
	const wstring& body) const
{
	http_request request = CreateRequest(
		methods::POST,
		RESOURCE_PATH_SPROCS,
		this->resource_id(),
		this->document_db_configuration()->master_key());
	request.set_request_uri(this->self() + sprocs_);

	value body_;
	body_[DOCUMENT_ID] = value::string(id);
	body_[BODY] = value::string(body);

	request.set_body(body_);

	return this->document_db_configuration()->http_client().request(request).then([=](http_response response)
	{
		value json_response = response.extract_json().get();

		if (response.status_code() == status_codes::Created)
		{
			return StoredProcedureFromJson(&json_response);
		}

		ThrowExceptionFromResponse(response.status_code(), json_response);
	});
}

shared_ptr<StoredProcedure> Collection::CreateStoredProcedure(
	const wstring& id,
	const wstring& body) const
{
	return CreateStoredProcedureAsync(id, body).get();
}

Concurrency::task<shared_ptr<StoredProcedure>> Collection::GetStoredProcedureAsync(
	const wstring& resource_id) const
{
	http_request request = CreateRequest(
		methods::GET,
		RESOURCE_PATH_SPROCS,
		resource_id,
		this->document_db_configuration()->master_key());
	request.set_request_uri(this->self() + sprocs_ + resource_id);

	return this->document_db_configuration()->http_client().request(request).then([=](http_response response)
	{
		value json_response = response.extract_json().get();

		if (response.status_code() == status_codes::OK)
		{
			return StoredProcedureFromJson(&json_response);
		}

		ThrowExceptionFromResponse(response.status_code(), json_response);
	});
}

shared_ptr<StoredProcedure> Collection::GetStoredProcedure(
	const wstring& resource_id) const
{
	return GetStoredProcedureAsync(resource_id).get();
}

Concurrency::task<vector<shared_ptr<StoredProcedure>>> Collection::ListStoredProceduresAsync() const
{
	http_request request = CreateRequest(
		methods::GET,
		RESOURCE_PATH_SPROCS,
		this->resource_id(),
		this->document_db_configuration()->master_key());
	request.set_request_uri(this->self() + sprocs_);
	return this->document_db_configuration()->http_client().request(request).then([=](http_response response)
	{
		value json_response = response.extract_json().get();

		if (response.status_code() == status_codes::OK)
		{
			assert(this->resource_id() == json_response.at(RESPONSE_RESOURCE_RID).as_string());
			vector<shared_ptr<StoredProcedure>> storedProcedures;
			storedProcedures.reserve(json_response.at(RESPONSE_BODY_COUNT).as_integer());
			value json_sprocs = json_response.at(RESPONSE_QUERY_SPROCS);

			for (auto iter = json_sprocs.as_array().cbegin(); iter != json_sprocs.as_array().cend(); ++iter)
			{
				shared_ptr<StoredProcedure> coll = StoredProcedureFromJson(&(*iter));
				storedProcedures.push_back(coll);
			}
			return storedProcedures;
		}

		ThrowExceptionFromResponse(response.status_code(), json_response);
	});
}

vector<shared_ptr<StoredProcedure>> Collection::ListStoredProcedures() const
{
	return ListStoredProceduresAsync().get();
}

Concurrency::task<shared_ptr<StoredProcedure>> Collection::ReplaceStoredProcedureAsync(
	const wstring& id,
	const wstring& new_id,
	const wstring& body) const
{
	http_request request = CreateRequest(
		methods::PUT,
		RESOURCE_PATH_SPROCS,
		id,
		this->document_db_configuration()->master_key());
	request.set_request_uri(this->self() + sprocs_ + id);

	value body_;
	body_[DOCUMENT_ID] = value::string(new_id);
	body_[BODY] = value::string(body);

	request.set_body(body_);

	return this->document_db_configuration()->http_client().request(request).then([=](http_response response)
	{
		value json_response = response.extract_json().get();

		if (response.status_code() == status_codes::OK)
		{
			return StoredProcedureFromJson(&json_response);
		}

		ThrowExceptionFromResponse(response.status_code(), json_response);
	});
}

shared_ptr<StoredProcedure> Collection::ReplaceStoredProcedure(
	const wstring& id,
	const wstring& new_id,
	const wstring& body) const
{
	return this->ReplaceStoredProcedureAsync(id, new_id, body).get();
}

Concurrency::task<void> Collection::DeleteStoredProcedureAsync(
	const shared_ptr<StoredProcedure>& storedProcedure) const
{
	return DeleteStoredProcedureAsync(storedProcedure->resource_id());
}

void Collection::DeleteStoredProcedure(
	const shared_ptr<StoredProcedure>& storedProcedure) const
{
	DeleteStoredProcedureAsync(storedProcedure->resource_id()).get();
}

Concurrency::task<void> Collection::DeleteStoredProcedureAsync(
	const wstring& resource_id) const
{
	http_request request = CreateRequest(
		methods::DEL,
		RESOURCE_PATH_SPROCS,
		resource_id,
		this->document_db_configuration()->master_key());
	request.set_request_uri(this->self() + sprocs_ + resource_id);

	return this->document_db_configuration()->http_client().request(request).then([=](http_response response)
	{
		if (response.status_code() == status_codes::NoContent)
		{
			return;
		}

		value json_response = response.extract_json().get();
		ThrowExceptionFromResponse(response.status_code(), json_response);
	});
}

void Collection::DeleteStoredProcedure(
	const wstring& resource_id) const
{
	DeleteStoredProcedureAsync(resource_id).get();
}

Concurrency::task<shared_ptr<StoredProcedureIterator>> Collection::QueryStoredProceduresAsync(
	const wstring& query,
	const int page_size) const
{
	http_request request = CreateQueryRequest(
		query,
		page_size,
		RESOURCE_PATH_SPROCS,
		this->resource_id(),
		this->document_db_configuration()->master_key());
	const wstring requestUri = this->self() + sprocs_;
	request.set_request_uri(requestUri);

	return this->document_db_configuration()->http_client().request(request).then([=](http_response response)
	{
		wstring continuation_id = response.headers()[HEADER_MS_CONTINUATION];
		value json_response = response.extract_json().get();

		if (response.status_code() == status_codes::OK)
		{
			assert(this->resource_id() == json_response.at(RESPONSE_RESOURCE_RID).as_string());


			return make_shared<StoredProcedureIterator>(
				shared_from_this(),
				query,
				page_size,
				requestUri,
				continuation_id,
				json_response.at(RESPONSE_QUERY_SPROCS));
		}

		ThrowExceptionFromResponse(response.status_code(), json_response);
	});
}

shared_ptr<StoredProcedureIterator> Collection::QueryStoredProcedures(
	const wstring& query,
	const int page_size) const
{
	return QueryStoredProceduresAsync(query, page_size).get();
}

Concurrency::task<void> Collection::ExecuteStoredProcedureAsync(
	const std::wstring& resource_id,
	const value& input) const
{
	http_request request = CreateRequest(
		methods::POST,
		RESOURCE_PATH_SPROCS,
		resource_id,
		this->document_db_configuration()->master_key());
	request.set_request_uri(this->self() + sprocs_ + resource_id);

	request.set_body(input);

	return this->document_db_configuration()->http_client().request(request).then([=](http_response response)
	{
		if (response.status_code() == status_codes::OK)
		{
			return;
		}

		value json_response = response.extract_json().get();
		ThrowExceptionFromResponse(response.status_code(), json_response);
	});
}

void Collection::ExecuteStoredProcedure(
	const std::wstring& resource_id,
	const value& input) const
{
	ExecuteStoredProcedureAsync(resource_id, input).get();
}

Concurrency::task<std::shared_ptr<UserDefinedFunction>> Collection::CreateUserDefinedFunctionAsync(
	const std::wstring& id,
	const std::wstring& body) const
{
	http_request request = CreateRequest(
		methods::POST,
		RESOURCE_PATH_UDFS,
		this->resource_id(),
		this->document_db_configuration()->master_key());
	request.set_request_uri(this->self() + udfs_);

	value body_;
	body_[DOCUMENT_ID] = value::string(id);
	body_[BODY] = value::string(body);

	request.set_body(body_);

	return this->document_db_configuration()->http_client().request(request).then([=](http_response response)
	{
		value json_response = response.extract_json().get();

		if (response.status_code() == status_codes::Created)
		{
			return UserDefinedFunctionFromJson(&json_response);
		}

		ThrowExceptionFromResponse(response.status_code(), json_response);
	});
}

std::shared_ptr<UserDefinedFunction> Collection::CreateUserDefinedFunction(
	const std::wstring& id,
	const std::wstring& body) const
{
	return CreateUserDefinedFunctionAsync(id, body).get();
}

Concurrency::task<std::shared_ptr<UserDefinedFunction>> Collection::GetUserDefinedFunctionAsync(
	const std::wstring& resource_id) const
{
	http_request request = CreateRequest(
		methods::GET,
		RESOURCE_PATH_UDFS,
		resource_id,
		this->document_db_configuration()->master_key());
	request.set_request_uri(this->self() + udfs_ + resource_id);

	return this->document_db_configuration()->http_client().request(request).then([=](http_response response)
	{
		value json_response = response.extract_json().get();

		if (response.status_code() == status_codes::OK)
		{
			return UserDefinedFunctionFromJson(&json_response);
		}

		ThrowExceptionFromResponse(response.status_code(), json_response);
	});
}

std::shared_ptr<UserDefinedFunction> Collection::GetUserDefinedFunction(
	const std::wstring& resource_id) const
{
	return GetUserDefinedFunctionAsync(resource_id).get();
}

Concurrency::task<std::vector<std::shared_ptr<UserDefinedFunction>>> Collection::ListUserDefinedFunctionsAsync() const
{
	http_request request = CreateRequest(
		methods::GET,
		RESOURCE_PATH_UDFS,
		this->resource_id(),
		this->document_db_configuration()->master_key());
	request.set_request_uri(this->self() + udfs_);
	return this->document_db_configuration()->http_client().request(request).then([=](http_response response)
	{
		value json_response = response.extract_json().get();

		if (response.status_code() == status_codes::OK)
		{
			assert(this->resource_id() == json_response.at(RESPONSE_RESOURCE_RID).as_string());
			vector<shared_ptr<UserDefinedFunction>> userDefinedFunctions;
			userDefinedFunctions.reserve(json_response.at(RESPONSE_BODY_COUNT).as_integer());
			value json_udfs = json_response.at(RESPONSE_QUERY_UDFS);

			for (auto iter = json_udfs.as_array().cbegin(); iter != json_udfs.as_array().cend(); ++iter)
			{
				shared_ptr<UserDefinedFunction> coll = UserDefinedFunctionFromJson(&(*iter));
				userDefinedFunctions.push_back(coll);
			}
			return userDefinedFunctions;
		}

		ThrowExceptionFromResponse(response.status_code(), json_response);
	});
}

std::vector<std::shared_ptr<UserDefinedFunction>> Collection::ListUserDefinedFunctions() const
{
	return ListUserDefinedFunctionsAsync().get();
}

Concurrency::task<std::shared_ptr<UserDefinedFunction>> Collection::ReplaceUserDefinedFunctionAsync(
	const std::wstring& id,
	const std::wstring& new_id,
	const std::wstring& body) const
{
	http_request request = CreateRequest(
		methods::PUT,
		RESOURCE_PATH_UDFS,
		id,
		this->document_db_configuration()->master_key());
	request.set_request_uri(this->self() + udfs_ + id);

	value body_;
	body_[DOCUMENT_ID] = value::string(new_id);
	body_[BODY] = value::string(body);

	request.set_body(body_);

	return this->document_db_configuration()->http_client().request(request).then([=](http_response response)
	{
		value json_response = response.extract_json().get();

		if (response.status_code() == status_codes::OK)
		{
			return UserDefinedFunctionFromJson(&json_response);
		}

		ThrowExceptionFromResponse(response.status_code(), json_response);
	});
}

std::shared_ptr<UserDefinedFunction> Collection::ReplaceUserDefinedFunction(
	const std::wstring& id,
	const std::wstring& new_id,
	const std::wstring& body) const
{
	return ReplaceUserDefinedFunctionAsync(id, new_id, body).get();
}

Concurrency::task<void> Collection::DeleteUserDefinedFunctionAsync(
	const std::shared_ptr<UserDefinedFunction>& userDefinedFunction) const
{
	return DeleteUserDefinedFunctionAsync(userDefinedFunction->resource_id());
}

void Collection::DeleteUserDefinedFunction(
	const std::shared_ptr<UserDefinedFunction>& userDefinedFunction) const
{
	return DeleteUserDefinedFunctionAsync(userDefinedFunction->resource_id()).get();
}

Concurrency::task<void> Collection::DeleteUserDefinedFunctionAsync(
	const std::wstring& resource_id) const
{
	http_request request = CreateRequest(
		methods::DEL,
		RESOURCE_PATH_UDFS,
		resource_id,
		this->document_db_configuration()->master_key());
	request.set_request_uri(this->self() + udfs_ + resource_id);

	return this->document_db_configuration()->http_client().request(request).then([=](http_response response)
	{
		if (response.status_code() == status_codes::NoContent)
		{
			return;
		}

		value json_response = response.extract_json().get();
		ThrowExceptionFromResponse(response.status_code(), json_response);
	});
}

void Collection::DeleteUserDefinedFunction(
	const std::wstring& resource_id) const
{
	return DeleteUserDefinedFunctionAsync(resource_id).get();
}

Concurrency::task<std::shared_ptr<UserDefinedFunctionIterator>> Collection::QueryUserDefinedFunctionsAsync(
	const std::wstring& query,
	const int page_size) const
{
	http_request request = CreateQueryRequest(
		query,
		page_size,
		RESOURCE_PATH_UDFS,
		this->resource_id(),
		this->document_db_configuration()->master_key());
	const wstring requestUri = this->self() + udfs_;
	request.set_request_uri(requestUri);

	return this->document_db_configuration()->http_client().request(request).then([=](http_response response)
	{
		wstring continuation_id = response.headers()[HEADER_MS_CONTINUATION];
		value json_response = response.extract_json().get();

		if (response.status_code() == status_codes::OK)
		{
			assert(this->resource_id() == json_response.at(RESPONSE_RESOURCE_RID).as_string());


			return make_shared<UserDefinedFunctionIterator>(
				shared_from_this(),
				query,
				page_size,
				requestUri,
				continuation_id,
				json_response.at(RESPONSE_QUERY_UDFS));
		}

		ThrowExceptionFromResponse(response.status_code(), json_response);
	});
}

std::shared_ptr<UserDefinedFunctionIterator> Collection::QueryUserDefinedFunctions(
	const std::wstring& query,
	const int page_size) const
{
	return QueryUserDefinedFunctionsAsync(query, page_size).get();
}
