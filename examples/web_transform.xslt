<xsl:stylesheet
  version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:xs="http://www.w3.org/2001/XMLSchema"
  exclude-result-prefixes="xs">

  <xsl:output method="html" indent="yes" encoding="UTF-8" />

  <xsl:template match="/">
    <html>
      <head>
        <title>Web commands</title>
        <style type="text/css">
          details summary { font-size : 30px; color: navy;}
          details { margin-top: 1em; }
          h3 { display: inline; margin-right: 10; }
          textarea { width: 100%; border: 1px solid #ccc; border-radius: 8px;}
        </style>
    <script src="https://cdn.jsdelivr.net/npm/@hpcc-js/wasm/dist/index.min.js"></script>
    <script>
        var hpccWasm = window["@hpcc-js/wasm"];
    </script>
      </head>
      <body>
        <div id="cmds">
          <h1 id="title">Web commands </h1>
          <xsl:apply-templates/>

        </div>
      </body>
    </html>
  </xsl:template>


  <xsl:template match="/commands/command">
    <details>
      <xsl:apply-templates select="name"/>
      <xsl:apply-templates select="description"/>
      <xsl:apply-templates select="request"/>
      <xsl:apply-templates select="response"/>
      <xsl:apply-templates select="requestGraph"/>
      <xsl:apply-templates select="responseGraph"/>
    </details>
  </xsl:template>


  <xsl:template match="/commands/command/name">
    <summary>
      <xsl:value-of select="."/>
    </summary>
  </xsl:template>

  <xsl:template match="/commands/command/description">
    <div><h3>Description </h3>
      <xsl:value-of select="."/>
    </div>
  </xsl:template>

  <xsl:template match="/commands/command/request">
    <xsl:if test=". and . !=''">
      <div><h3>Request </h3>
        <xsl:value-of select="."/>
      </div>
    </xsl:if>
  </xsl:template>

  <xsl:template match="/commands/command/response">
    <xsl:if test=". and . !=''">
      <div><h3>Response </h3>
        <xsl:value-of select="."/>
      </div>
    </xsl:if>
  </xsl:template>

  <xsl:template match="/commands/command/requestGraph">
    <xsl:if test=". and . !=''">
      <div id="ig{position()-1}"><h3>Sample request</h3><br/>
        <script>
        hpccWasm.graphvizSync().then(graphviz => {
            const div = document.getElementById('ig<xsl:value-of select="position()-1"/>');
            // Synchronous call to layout
            div.innerHTML += graphviz.layout('<xsl:value-of select="."/>', "svg", "dot");
        });
        </script>
      </div>
    </xsl:if>
  </xsl:template>

  <xsl:template match="/commands/command/responseGraph">
    <xsl:if test=". and . !=''">
      <div id="#og{position()-1}"><h3>Sample response</h3><br/>
        <script>
        hpccWasm.graphvizSync().then(graphviz => {
            const div = document.getElementById('#og<xsl:value-of select="position()-1"/>');
            // Synchronous call to layout
            div.innerHTML += graphviz.layout('<xsl:value-of select="."/>', "svg", "dot");
        });
        </script>
      </div>
    </xsl:if>
  </xsl:template>

</xsl:stylesheet>
