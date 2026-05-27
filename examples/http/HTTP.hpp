#pragma once

#include <PacketForge/macros/CommandSuit.hpp>
#include <PacketForge/macros/PacketStructure.hpp>
#include <PacketForge/serializers/IntSerializers.hpp>
#include <PacketForge/serializers/StringSerializers.hpp>
#include <PacketForge/serializers/VectorSerializers.hpp>
#include <PacketForge/serializers/MapSerializers.hpp>
#include <PacketForge/serializers/OptionalSerializers.hpp>
#include <PacketForge/serializers/VariantSerializers.hpp>

#include <optional>
#include <vector>
#include <string>
#include <map>
#include <array>
#include <cstdint>
#include <variant>

// ============================================================================
// БАЗОВЫЕ ТИПЫ И ПЕРЕЧИСЛЕНИЯ
// ============================================================================

enum class HttpMethod : uint8_t {
    GET     = 0, POST = 1, PUT = 2, DELETE = 3,
    PATCH   = 4, HEAD = 5, OPTIONS = 6, CONNECT = 7, TRACE = 8
};

enum class HttpVersion : uint8_t {
    Http10 = 0, Http11 = 1, Http2 = 2, Http3 = 3
};

enum class HttpStatusCode : uint16_t {
    // 1xx Informational
    Continue            = 100, SwitchingProtocols = 101,
    // 2xx Success
    OK                  = 200, Created = 201, Accepted = 202, NoContent = 204,
    // 3xx Redirection
    MovedPermanently    = 301, Found = 302, SeeOther = 303, NotModified = 304,
    // 4xx Client Error
    BadRequest          = 400, Unauthorized = 401, Forbidden = 403,
    NotFound            = 404, MethodNotAllowed = 405, Conflict = 409,
    UnprocessableEntity = 422, TooManyRequests = 429,
    // 5xx Server Error
    InternalServerError = 500, NotImplemented = 501, BadGateway = 502,
    ServiceUnavailable  = 503, GatewayTimeout = 504
};

enum class ContentType : uint8_t {
    None            = 0,
    TextPlain       = 1, TextHtml = 2, TextJson = 3, TextXml = 4,
    AppJson         = 5, AppXml = 6, AppForm = 7, AppOctetStream = 8,
    MultipartForm   = 9,
    Custom          = 255  // для кастомных типов через строку
};

enum class Compression : uint8_t {
    None = 0, Gzip = 1, Deflate = 2, Brotli = 3, Zstd = 4
};

// ============================================================================
// УНИВЕРСАЛЬНЫЕ ВСПОМОГАТЕЛЬНЫЕ СТРУКТУРЫ
// ============================================================================

// Ключ-значение для заголовков / параметров / куки
struct KeyValue {
    std::string key;    // [1..64]
    std::string value;  // [0..4096]
};
PACKET_STRUCTURE(KeyValue, &KeyValue::key, &KeyValue::value)

// Пагинация — переиспользуемый компонент
struct Pagination {
    std::optional<uint32_t> limit;    // [1..1000], default=50
    std::optional<uint32_t> offset;   // default=0
    std::optional<std::string> cursor; // для cursor-based пагинации
};
PACKET_STRUCTURE(Pagination,
    &Pagination::limit,
    &Pagination::offset,
    &Pagination::cursor)

// Сортировка: field + direction
struct SortOption {
    std::string field;      // имя поля
    bool ascending = true;  // направление
};
PACKET_STRUCTURE(SortOption, &SortOption::field, &SortOption::ascending)

// Фильтр: логика на уровне приложения (JSON/DSL строка)
struct Filter {
    std::string expression;  // например: "age>18 AND status='active'"
    std::map<std::string, std::string> params;  // переменные для подстановки
};
PACKET_STRUCTURE(Filter, &Filter::expression, &Filter::params)

// Мета-информация для трассировки и отладки
struct RequestContext {
    std::string correlation_id;  // UUID для tracing
    std::string client_ip;        // [7..45] для IPv4/IPv6
    std::optional<std::string> user_agent;
    std::optional<uint64_t> request_start_ms;  // timestamp
};
PACKET_STRUCTURE(RequestContext,
    &RequestContext::correlation_id,
    &RequestContext::client_ip,
    &RequestContext::user_agent,
    &RequestContext::request_start_ms)

// ============================================================================
// HTTP-ЗАПРОС
// ============================================================================

struct HttpRequest {
    // === Обязательные поля ===
    HttpMethod method;
    HttpVersion version;
    std::string path;  // [1..512], например "/api/v1/users"
    
    // === Опциональные компоненты ===
    std::map<std::string, std::vector<std::string>> query_params;  // multi-value: ?tag=a&tag=b
    std::map<std::string, std::string> headers;                     // стандартные заголовки
    std::map<std::string, std::string> cookies;                     // client cookies
    
    // === Тело запроса ===
    ContentType content_type = ContentType::None;
    Compression content_encoding = Compression::None;
    std::optional<std::vector<uint8_t>> body;  // бинарное тело
    
    // === Мета-данные ===
    std::optional<RequestContext> context;  // tracing, отладка
    std::optional<Pagination> pagination;   // для list-эндпоинтов
    std::optional<Filter> filter;           // для фильтрации
    std::vector<SortOption> sort;           // сортировка (0..N полей)
    
    // === Расширения (future-proof) ===
    std::map<std::string, std::string> extensions;  // кастомные поля
};
PACKET_STRUCTURE(HttpRequest,
    &HttpRequest::method,
    &HttpRequest::version,
    &HttpRequest::path,
    &HttpRequest::query_params,
    &HttpRequest::headers,
    &HttpRequest::cookies,
    &HttpRequest::content_type,
    &HttpRequest::content_encoding,
    &HttpRequest::body,
    &HttpRequest::context,
    &HttpRequest::pagination,
    &HttpRequest::filter,
    &HttpRequest::sort,
    &HttpRequest::extensions)

// ============================================================================
// HTTP-ОТВЕТ
// ============================================================================

struct HttpResponse {
    // === Status line ===
    HttpVersion version;
    HttpStatusCode status;
    std::string reason;  // опционально: "OK", "Not Found"
    
    // === Headers & metadata ===
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> set_cookies;  // Set-Cookie ответы
    std::optional<RequestContext> context;  // echo + server-side tracing
    
    // === Body ===
    ContentType content_type = ContentType::None;
    Compression content_encoding = Compression::None;
    std::optional<std::vector<uint8_t>> body;
    
    // === Performance & caching ===
    std::optional<uint64_t> server_time_ms;    // время обработки
    std::optional<uint64_t> cache_ttl_seconds; // для CDN/proxy
    std::optional<std::string> etag;           // для conditional requests
    std::optional<std::string> last_modified;  // RFC 7232
    
    // === Расширения ===
    std::map<std::string, std::string> extensions;
};
PACKET_STRUCTURE(HttpResponse,
    &HttpResponse::version,
    &HttpResponse::status,
    &HttpResponse::reason,
    &HttpResponse::headers,
    &HttpResponse::set_cookies,
    &HttpResponse::context,
    &HttpResponse::content_type,
    &HttpResponse::content_encoding,
    &HttpResponse::body,
    &HttpResponse::server_time_ms,
    &HttpResponse::cache_ttl_seconds,
    &HttpResponse::etag,
    &HttpResponse::last_modified,
    &HttpResponse::extensions)

// ============================================================================
// УНИВЕРСАЛЬНЫЕ ТИПЫ ДАННЫХ (для бизнес-логики)
// ============================================================================

// Универсальный ID: может быть числом или UUID-строкой
using ResourceId = std::variant<uint64_t, std::string>;

// Money с валютой (демонстрация вложенных структур)
struct Money {
    int64_t amount;           // в минорных единицах (центы, копейки)
    std::string currency;     // ISO 4217: "USD", "EUR", "RUB"
    uint8_t decimals = 2;     // точность
};
PACKET_STRUCTURE(Money, &Money::amount, &Money::currency, &Money::decimals)

// Гео-координаты
struct GeoPoint {
    double latitude;   // [-90.0, 90.0]
    double longitude;  // [-180.0, 180.0]
    std::optional<float> altitude;  // метры
};
PACKET_STRUCTURE(GeoPoint, &GeoPoint::latitude, &GeoPoint::longitude, &GeoPoint::altitude)

// Временной диапазон
struct TimeRange {
    uint64_t from;  // Unix timestamp ms
    uint64_t to;    // Unix timestamp ms
    std::optional<std::string> timezone;  // IANA: "Europe/Moscow"
};
PACKET_STRUCTURE(TimeRange, &TimeRange::from, &TimeRange::to, &TimeRange::timezone)

// ============================================================================
// ПРИКЛАДНЫЕ ЗАПРОСЫ (Command Suit: request)
// ============================================================================

DEFINE_DEFAULT_COMMAND_SUIT(api_request,
    // Auth
    Authenticate, RefreshToken, Logout,
    // Users
    ListUsers, GetUser, CreateUser, UpdateUser, DeleteUser,
    // Resources
    ListResources, GetResource, CreateResource, UpdateResource, DeleteResource,
    // System
    HealthCheck, GetMetrics, GetConfig,
    // Batch operations
    BatchCreate, BatchUpdate, BatchDelete,
)

// --- AUTH ---
struct Authenticate {
    std::string identifier;  // email или username
    std::string credential;  // password или token
    bool remember_me = false;
    std::optional<std::string> mfa_code;  // 2FA
};
PACKET_STRUCTURE(Authenticate,
    &Authenticate::identifier,
    &Authenticate::credential,
    &Authenticate::remember_me,
    &Authenticate::mfa_code)

struct RefreshToken {
    std::string refresh_token;
    std::optional<std::string> scope;  // ограничение прав нового токена
};
PACKET_STRUCTURE(RefreshToken, &RefreshToken::refresh_token, &RefreshToken::scope)

struct Logout {
    std::optional<bool> all_sessions = false;  // logout везде или только текущая
};
PACKET_STRUCTURE(Logout, &Logout::all_sessions)

// --- USERS ---
struct User {
    uint64_t id;
    std::string username;       // [3..32]
    std::string email;          // validated
    std::optional<std::string> display_name;
    std::optional<std::string> avatar_url;
    std::map<std::string, std::string> metadata;  // кастомные поля
    uint64_t created_at;
    std::optional<uint64_t> updated_at;
    std::optional<uint64_t> last_seen_at;
};
PACKET_STRUCTURE(User,
    &User::id, &User::username, &User::email,
    &User::display_name, &User::avatar_url,
    &User::metadata, &User::created_at,
    &User::updated_at, &User::last_seen_at)

struct ListUsers {
    Pagination pagination;
    std::optional<Filter> filter;
    std::vector<SortOption> sort;
    std::optional<std::vector<std::string>> fields;  // field selection: ["id","username"]
};
PACKET_STRUCTURE(ListUsers,
    &ListUsers::pagination,
    &ListUsers::filter,
    &ListUsers::sort,
    &ListUsers::fields)

struct GetUser {
    ResourceId user_id;  // variant: number or string
    std::optional<std::vector<std::string>> fields;
};
PACKET_STRUCTURE(GetUser, &GetUser::user_id, &GetUser::fields)

struct CreateUser {
    std::string username;
    std::string email;
    std::string password_hash;  // хеш, не пароль!
    std::optional<std::string> display_name;
    std::map<std::string, std::string> metadata;
};
PACKET_STRUCTURE(CreateUser,
    &CreateUser::username,
    &CreateUser::email,
    &CreateUser::password_hash,
    &CreateUser::display_name,
    &CreateUser::metadata)

struct UpdateUser {
    ResourceId user_id;
    // Только non-null поля обновляются — сила optional!
    std::optional<std::string> email;
    std::optional<std::string> display_name;
    std::optional<std::string> avatar_url;
    std::optional<std::map<std::string, std::string>> metadata;  // merge or replace?
    std::optional<bool> metadata_merge = true;  // если true — merge, иначе replace
};
PACKET_STRUCTURE(UpdateUser,
    &UpdateUser::user_id,
    &UpdateUser::email,
    &UpdateUser::display_name,
    &UpdateUser::avatar_url,
    &UpdateUser::metadata,
    &UpdateUser::metadata_merge)

struct DeleteUser {
    ResourceId user_id;
    bool permanent = false;  // soft/hard delete
    std::optional<std::string> reason;  // для аудита
};
PACKET_STRUCTURE(DeleteUser, &DeleteUser::user_id, &DeleteUser::permanent, &DeleteUser::reason)

// --- RESOURCES (универсальный CRUD) ---
struct Resource {
    ResourceId id;
    std::string type;           // "article", "product", etc.
    std::map<std::string, std::string> attributes;  // динамические поля
    std::map<std::string, std::vector<std::string>> relationships;  // связи
    std::optional<GeoPoint> location;
    std::optional<Money> price;
    uint64_t created_at;
};
PACKET_STRUCTURE(Resource,
    &Resource::id, &Resource::type, &Resource::attributes,
    &Resource::relationships, &Resource::location,
    &Resource::price, &Resource::created_at)

struct ListResources {
    std::string resource_type;
    Pagination pagination;
    std::optional<Filter> filter;
    std::vector<SortOption> sort;
    std::optional<TimeRange> time_range;  // created_at filter
};
PACKET_STRUCTURE(ListResources,
    &ListResources::resource_type,
    &ListResources::pagination,
    &ListResources::filter,
    &ListResources::sort,
    &ListResources::time_range)

struct GetResource {
    std::string resource_type;
    ResourceId resource_id;
};
PACKET_STRUCTURE(GetResource, &GetResource::resource_type, &GetResource::resource_id)

struct CreateResource {
    std::string resource_type;
    std::map<std::string, std::string> attributes;
    std::map<std::string, std::vector<std::string>> relationships;
    std::optional<GeoPoint> location;
    std::optional<Money> price;
};
PACKET_STRUCTURE(CreateResource,
    &CreateResource::resource_type,
    &CreateResource::attributes,
    &CreateResource::relationships,
    &CreateResource::location,
    &CreateResource::price)

struct UpdateResource {
    std::string resource_type;
    ResourceId resource_id;
    std::map<std::string, std::string> attributes;  // только изменяемые
    std::optional<bool> merge_attributes = true;
};
PACKET_STRUCTURE(UpdateResource,
    &UpdateResource::resource_type,
    &UpdateResource::resource_id,
    &UpdateResource::attributes,
    &UpdateResource::merge_attributes)

struct DeleteResource {
    std::string resource_type;
    ResourceId resource_id;
    bool permanent = false;
};
PACKET_STRUCTURE(DeleteResource,
    &DeleteResource::resource_type,
    &DeleteResource::resource_id,
    &DeleteResource::permanent)

// --- SYSTEM ---
struct HealthCheck {
    std::optional<bool> deep = false;  // если true — проверка БД, кешей и т.д.
};
PACKET_STRUCTURE(HealthCheck, &HealthCheck::deep)

struct GetMetrics {
    std::optional<std::vector<std::string>> names;  // конкретные метрики
    std::optional<TimeRange> time_range;
    std::optional<std::string> format;  // "prometheus", "json", "protobuf"
};
PACKET_STRUCTURE(GetMetrics,
    &GetMetrics::names,
    &GetMetrics::time_range,
    &GetMetrics::format)

struct GetConfig {
    std::optional<std::vector<std::string>> sections;  // какие секции вернуть
    bool include_defaults = false;  // возвращать ли значения по умолчанию
};
PACKET_STRUCTURE(GetConfig, &GetConfig::sections, &GetConfig::include_defaults)

// --- BATCH OPERATIONS ---
template<typename T>
struct BatchOperation {
    std::vector<T> items;
    bool atomic = true;  // all-or-nothing
    std::optional<uint32_t> max_parallel = 4;  // лимит параллелизма на сервере
};
// Не используем PACKET_STRUCTURE для шаблонов — инстанцируем явно ниже

// Явные инстанциации для сериализации
using BatchCreateUsers = BatchOperation<CreateUser>;
using BatchUpdateUsers = BatchOperation<UpdateUser>;
using BatchDeleteResources = BatchOperation<DeleteResource>;

// Макрос для инстанцированных шаблонов
PACKET_STRUCTURE(BatchCreateUsers, &BatchCreateUsers::items, &BatchCreateUsers::atomic, &BatchCreateUsers::max_parallel)
PACKET_STRUCTURE(BatchUpdateUsers, &BatchUpdateUsers::items, &BatchUpdateUsers::atomic, &BatchUpdateUsers::max_parallel)
PACKET_STRUCTURE(BatchDeleteResources, &BatchDeleteResources::items, &BatchDeleteResources::atomic, &BatchDeleteResources::max_parallel)

// ============================================================================
// ПРИКЛАДНЫЕ ОТВЕТЫ (Command Suit: response)
// ============================================================================

DEFINE_DEFAULT_COMMAND_SUIT(api_response,
    // Auth
    AuthSuccess, AuthFailure, TokenRefreshed,
    // Users
    UsersList, UserDetail, UserCreated, UserUpdated, UserDeleted,
    // Resources
    ResourcesList, ResourceDetail, ResourceCreated, ResourceUpdated, ResourceDeleted,
    // System
    HealthReport, MetricsData, ConfigData,
    // Batch
    BatchResult,
    // Errors
    ApiError, ValidationError, RateLimitError,
)

// --- AUTH RESPONSES ---
struct AuthSuccess {
    uint64_t user_id;
    std::string access_token;
    std::string refresh_token;
    uint64_t expires_at;  // Unix timestamp
    std::vector<std::string> scopes;  // permissions
    std::optional<User> user_info;    // опционально вернуть профиль
};
PACKET_STRUCTURE(AuthSuccess,
    &AuthSuccess::user_id,
    &AuthSuccess::access_token,
    &AuthSuccess::refresh_token,
    &AuthSuccess::expires_at,
    &AuthSuccess::scopes,
    &AuthSuccess::user_info)

enum class AuthFailureReason : uint8_t {
    InvalidCredentials = 1, AccountDisabled = 2, MfaRequired = 3,
    TokenExpired = 4, TokenRevoked = 5, RateLimited = 6
};

struct AuthFailure {
    AuthFailureReason reason;
    std::string message;  // человекочитаемый, можно локализовать
    std::optional<uint64_t> retry_after_seconds;  // для rate limit
    std::optional<std::string> mfa_challenge;     // если нужен 2FA
};
PACKET_STRUCTURE(AuthFailure,
    &AuthFailure::reason,
    &AuthFailure::message,
    &AuthFailure::retry_after_seconds,
    &AuthFailure::mfa_challenge)

struct TokenRefreshed {
    std::string new_access_token;
    uint64_t expires_at;
    std::optional<std::string> new_refresh_token;  // ротация
};
PACKET_STRUCTURE(TokenRefreshed,
    &TokenRefreshed::new_access_token,
    &TokenRefreshed::expires_at,
    &TokenRefreshed::new_refresh_token)

// --- USER RESPONSES ---
struct UsersList {
    std::vector<User> users;
    uint32_t total;
    uint32_t returned;
    Pagination pagination;
    std::optional<std::string> next_cursor;  // для следующей страницы
};
PACKET_STRUCTURE(UsersList,
    &UsersList::users, &UsersList::total,
    &UsersList::returned, &UsersList::pagination,
    &UsersList::next_cursor)

struct UserDetail { User user; };
PACKET_STRUCTURE(UserDetail, &UserDetail::user)

struct UserCreated {
    uint64_t user_id;
    std::string location;  // URL ресурса: "/users/123"
    User user;  // полный объект или частично
};
PACKET_STRUCTURE(UserCreated, &UserCreated::user_id, &UserCreated::location, &UserCreated::user)

struct UserUpdated {
    uint64_t user_id;
    uint64_t updated_at;
    std::optional<User> user;  // если нужно вернуть обновлённый объект
};
PACKET_STRUCTURE(UserUpdated, &UserUpdated::user_id, &UserUpdated::updated_at, &UserUpdated::user)

struct UserDeleted {
    uint64_t user_id;
    bool permanent;
    uint64_t deleted_at;
};
PACKET_STRUCTURE(UserDeleted, &UserDeleted::user_id, &UserDeleted::permanent, &UserDeleted::deleted_at)

// --- RESOURCE RESPONSES ---
struct ResourcesList {
    std::string resource_type;
    std::vector<Resource> resources;
    uint32_t total;
    uint32_t returned;
    Pagination pagination;
    std::optional<std::string> next_cursor;
};
PACKET_STRUCTURE(ResourcesList,
    &ResourcesList::resource_type,
    &ResourcesList::resources, &ResourcesList::total,
    &ResourcesList::returned, &ResourcesList::pagination,
    &ResourcesList::next_cursor)

struct ResourceDetail { Resource resource; };
PACKET_STRUCTURE(ResourceDetail, &ResourceDetail::resource)

struct ResourceCreated {
    std::string resource_type;
    ResourceId resource_id;
    std::string location;
    Resource resource;
};
PACKET_STRUCTURE(ResourceCreated,
    &ResourceCreated::resource_type,
    &ResourceCreated::resource_id,
    &ResourceCreated::location,
    &ResourceCreated::resource)

struct ResourceUpdated {
    std::string resource_type;
    ResourceId resource_id;
    uint64_t updated_at;
    std::optional<Resource> resource;
};
PACKET_STRUCTURE(ResourceUpdated,
    &ResourceUpdated::resource_type,
    &ResourceUpdated::resource_id,
    &ResourceUpdated::updated_at,
    &ResourceUpdated::resource)

struct ResourceDeleted {
    std::string resource_type;
    ResourceId resource_id;
    bool permanent;
    uint64_t deleted_at;
};
PACKET_STRUCTURE(ResourceDeleted,
    &ResourceDeleted::resource_type,
    &ResourceDeleted::resource_id,
    &ResourceDeleted::permanent,
    &ResourceDeleted::deleted_at)

// --- SYSTEM RESPONSES ---
struct HealthReport {
    bool healthy;
    std::string version;
    uint64_t uptime_seconds;
    std::map<std::string, bool> checks;  // "database": true, "cache": false
    std::optional<std::string> details;
};
PACKET_STRUCTURE(HealthReport,
    &HealthReport::healthy, &HealthReport::version,
    &HealthReport::uptime_seconds, &HealthReport::checks,
    &HealthReport::details)

struct MetricPoint {
    std::string name;
    double value;
    uint64_t timestamp;
    std::map<std::string, std::string> labels;  // Prometheus-style
};
PACKET_STRUCTURE(MetricPoint,
    &MetricPoint::name, &MetricPoint::value,
    &MetricPoint::timestamp, &MetricPoint::labels)

struct MetricsData {
    std::vector<MetricPoint> points;
    std::optional<std::string> format;  // если конвертировали
};
PACKET_STRUCTURE(MetricsData, &MetricsData::points, &MetricsData::format)

struct ConfigData {
    std::map<std::string, std::map<std::string, std::string>> sections;
    uint64_t last_updated;
    std::string checksum;  // для cache validation
};
PACKET_STRUCTURE(ConfigData,
    &ConfigData::sections,
    &ConfigData::last_updated,
    &ConfigData::checksum)

// --- BATCH RESPONSE ---
struct BatchResultItem {
    size_t index;  // индекс в исходном запросе
    bool success;
    std::optional<ResourceId> created_id;  // если создавали
    std::optional<std::string> error;      // если ошибка
};
PACKET_STRUCTURE(BatchResultItem,
    &BatchResultItem::index,
    &BatchResultItem::success,
    &BatchResultItem::created_id,
    &BatchResultItem::error)

struct BatchResult {
    std::vector<BatchResultItem> results;
    size_t success_count;
    size_t error_count;
    bool atomic;  // был ли запрос атомарным
    std::optional<std::string> transaction_id;  // если поддерживаются транзакции
};
PACKET_STRUCTURE(BatchResult,
    &BatchResult::results,
    &BatchResult::success_count,
    &BatchResult::error_count,
    &BatchResult::atomic,
    &BatchResult::transaction_id)

// --- ERRORS ---
enum class ErrorSeverity : uint8_t {
    Info = 0, Warning = 1, Error = 2, Critical = 3
};

struct ErrorDetail {
    std::string field;        // какое поле проблемное
    std::string code;         // код ошибки: "REQUIRED", "INVALID_FORMAT"
    std::string message;      // описание
    std::optional<std::string> suggestion;  // как исправить
};
PACKET_STRUCTURE(ErrorDetail,
    &ErrorDetail::field,
    &ErrorDetail::code,
    &ErrorDetail::message,
    &ErrorDetail::suggestion)

struct ApiError {
    HttpStatusCode http_code;
    std::string error_code;        // машинный код: "USER_NOT_FOUND"
    std::string message;           // человекочитаемый
    ErrorSeverity severity = ErrorSeverity::Error;
    std::optional<std::string> documentation_url;
    std::optional<RequestContext> context;  // для отладки
};
PACKET_STRUCTURE(ApiError,
    &ApiError::http_code,
    &ApiError::error_code,
    &ApiError::message,
    &ApiError::severity,
    &ApiError::documentation_url,
    &ApiError::context)

struct ValidationError {
    HttpStatusCode http_code = HttpStatusCode::UnprocessableEntity;
    std::string error_code = "VALIDATION_FAILED";
    std::vector<ErrorDetail> errors;  // список проблем
    std::optional<std::map<std::string, std::string>> received_data;  // что пришло (для отладки)
};
PACKET_STRUCTURE(ValidationError,
    &ValidationError::http_code,
    &ValidationError::error_code,
    &ValidationError::errors,
    &ValidationError::received_data)

struct RateLimitError {
    HttpStatusCode http_code = HttpStatusCode::TooManyRequests;
    std::string error_code = "RATE_LIMITED";
    uint64_t retry_after_seconds;
    uint64_t limit;           // максимум запросов
    uint64_t remaining;       // осталось в окне
    uint64_t reset_at;        // когда сбросится (Unix timestamp)
    std::optional<std::string> scope;  // к чему применён лимит
};
PACKET_STRUCTURE(RateLimitError,
    &RateLimitError::http_code,
    &RateLimitError::error_code,
    &RateLimitError::retry_after_seconds,
    &RateLimitError::limit,
    &RateLimitError::remaining,
    &RateLimitError::reset_at,
    &RateLimitError::scope)
