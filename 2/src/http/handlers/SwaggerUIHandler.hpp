#pragma once
#include "../BaseHandler.hpp"

class SwaggerUIHandler : public BaseHandler
{
protected:
    void handle(Poco::Net::HTTPServerRequest &req,
                Poco::Net::HTTPServerResponse &res) override
    {
        res.setContentType("text/html");
        res.setStatus(Poco::Net::HTTPResponse::HTTP_OK);

        res.send() << R"(
<!DOCTYPE html>
<html>
<head>
  <title>Swagger UI</title>
  <link rel="stylesheet" href="https://unpkg.com/swagger-ui-dist/swagger-ui.css" />
</head>
<body>
  <div id="swagger-ui"></div>

  <script src="https://unpkg.com/swagger-ui-dist/swagger-ui-bundle.js"></script>
  <script>
    const ui = SwaggerUIBundle({
      url: '/swagger',
      dom_id: '#swagger-ui'
    });
  </script>
</body>
</html>
)";
    }
};