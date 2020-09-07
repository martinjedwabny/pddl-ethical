<?xml version="1.0"?>

<xsl:stylesheet
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:xs="http://www.w3.org/2001/XMLSchema"
    xmlns:op="http://www.w3.org/2002/04/xquery-operators"
    xmlns:fn="http://www.w3.org/2002/04/xquery-functions"
    xmlns:xhtml="http://www.w3.org/1999/xhtml"
    xmlns:svg="http://www.w3.org/2000/svg"
    xmlns:xlink="http://www.w3.org/1999/xlink"
    xmlns:src="http://dummy.net/pddlcat/schedule"
    version="2.0" >
  <xsl:output indent="yes" method="xml"/>

  <xsl:param name="chart-display-width" as="xs:decimal">800</xsl:param>
  <xsl:param name="chart-unit-size" as="xs:decimal">10</xsl:param>
  <xsl:param name="resource-profile-display-height" as="xs:decimal">100</xsl:param>
  <xsl:param name="list-step-order-traits">true</xsl:param>
  <xsl:param name="basename">plan</xsl:param>

  <xsl:template name="makeGanttChart">
    <xsl:param name="id">GanttChart</xsl:param>
    <xsl:param name="time-normalization-factor"/>

    <xsl:variable name="finish-times" as="xs:decimal*">
      <xsl:for-each select="src:steps/src:step">
        <xsl:sequence select="xs:decimal(@finish)"/>
      </xsl:for-each>
    </xsl:variable>
    <xsl:variable name="latest-finish-time" as="xs:decimal">
      <xsl:value-of select="max($finish-times)"/>
    </xsl:variable>
    <xsl:variable name="normalized-makespan" as="xs:decimal">
      <xsl:value-of select="$latest-finish-time div $time-normalization-factor"/>
    </xsl:variable>

    <xsl:variable name="number-of-tracks" as="xs:decimal">
      <xsl:value-of select="xs:decimal(src:steps/@maxtrack) + 1"/>
    </xsl:variable>

    <xsl:element name="svg:symbol">
      <xsl:attribute name="id">
        <xsl:value-of select="$id"/>
      </xsl:attribute>
      <xsl:attribute name="width">
	<xsl:value-of select="$normalized-makespan * $chart-unit-size"/>
      </xsl:attribute>
      <xsl:attribute name="height">
	<xsl:value-of select="$number-of-tracks * $chart-unit-size"/>
      </xsl:attribute>
      <xsl:for-each select="src:steps/src:step">
        <xsl:element name="svg:rect">
	  <xsl:attribute name="id">
	    <xsl:value-of select="concat('gc_step',@id)"/>
	  </xsl:attribute>
	  <xsl:attribute name="x">
	    <xsl:value-of select="(xs:decimal(@start) div $time-normalization-factor) * $chart-unit-size"/>
	  </xsl:attribute>
	  <xsl:attribute name="y">
	    <xsl:value-of select="xs:decimal(@track) * $chart-unit-size"/>
	  </xsl:attribute>
	  <xsl:attribute name="width">
	    <xsl:value-of select="(xs:decimal(@finish - @start) div $time-normalization-factor) * $chart-unit-size"/>
	  </xsl:attribute>
	  <xsl:attribute name="height">
	    <xsl:value-of select="10"/>
	  </xsl:attribute>
	  <!--
	  <xsl:attribute name="onmouseover">gantt_chart_mouse_in(<xsl:value-of select="@id"/>)</xsl:attribute>
	  <xsl:attribute name="onmouseout">gantt_chart_mouse_out(<xsl:value-of select="@id"/>)</xsl:attribute>
	  -->
	  <xsl:attribute name="onclick">gantt_chart_mouse_click(<xsl:value-of select="@id"/>)</xsl:attribute>
	</xsl:element>
      </xsl:for-each>
    </xsl:element>
  </xsl:template>


  <xsl:template name="makeTimeGrid">
    <xsl:param name="id">TimeGrid</xsl:param>
    <xsl:param name="time-normalization-factor"/>
    <xsl:element name="svg:symbol">
      <xsl:attribute name="id">
        <xsl:value-of select="$id"/>
      </xsl:attribute>
      <xsl:for-each-group select="src:steps/src:step" group-by="@start">
        <xsl:element name="svg:path">
	  <xsl:attribute name="d"><xsl:text>M </xsl:text><xsl:value-of select="(xs:decimal(@start) div $time-normalization-factor) * $chart-unit-size"/><xsl:text> 0 L </xsl:text><xsl:value-of select="(xs:decimal(@start) div $time-normalization-factor) * $chart-unit-size"/><xsl:text> </xsl:text><xsl:value-of select="$chart-unit-size"/></xsl:attribute>
	</xsl:element>
      </xsl:for-each-group>
    </xsl:element>
  </xsl:template>


  <xsl:template name="makeProfile3">
    <xsl:param name="id">Profile</xsl:param>
    <xsl:param name="time-normalization-factor"/>
    <xsl:param name="avail"/>
    <xsl:param name="in_use"/>
    <xsl:param name="init-level"/>
    <xsl:variable name="level-normalization-factor" as="xs:decimal">
      <xsl:value-of select="$init-level div $resource-profile-display-height"/>
    </xsl:variable>
    <xsl:element name="svg:symbol">
      <xsl:attribute name="id">
        <xsl:value-of select="$id"/>
      </xsl:attribute>
      <xsl:for-each select="$in_use/src:interval">
        <xsl:variable name="i"><xsl:value-of select="position()"/></xsl:variable>
	<xsl:variable name="l1" as="xs:decimal"><xsl:value-of select="xs:decimal($in_use/src:interval[position()=$i]/@amount)"/></xsl:variable>
	<xsl:variable name="l2" as="xs:decimal"><xsl:value-of select="xs:decimal($avail/src:interval[position()=$i]/@amount)"/></xsl:variable>
<!--
	<xsl:message>
debug: interval <xsl:value-of select="$i"/>: l1 = <xsl:value-of select="$l1"/>, l2 = <xsl:value-of select="$l2"/>
	</xsl:message>
-->
	<xsl:if test="$l1 &gt; 0">
<!--
	  <xsl:message>
debug: yellow rect with x = <xsl:value-of select="(xs:decimal($in_use/src:interval[position()=$i]/@start) div $time-normalization-factor) * $chart-unit-size"/>, width = <xsl:value-of select="((xs:decimal($in_use/src:interval[position()=$i]/@end) - xs:decimal($in_use/src:interval[position()=$i]/@start)) div $time-normalization-factor) * $chart-unit-size"/>, y = <xsl:value-of select="($init-level - $l1) div $level-normalization-factor"/>, height = <xsl:value-of select="$l1 div $level-normalization-factor"/>.
	  </xsl:message>
-->
	  <xsl:element name="svg:rect">
	    <xsl:attribute name="x">
	      <xsl:value-of select="(xs:decimal($in_use/src:interval[position()=$i]/@start) div $time-normalization-factor) * $chart-unit-size"/>
	    </xsl:attribute>
	    <xsl:attribute name="width">
	      <xsl:value-of select="((xs:decimal($in_use/src:interval[position()=$i]/@end) - xs:decimal($in_use/src:interval[position()=$i]/@start)) div $time-normalization-factor) * $chart-unit-size"/>
	    </xsl:attribute>
	    <xsl:attribute name="y">
	      <xsl:value-of select="($init-level - $l1) div $level-normalization-factor"/>
	    </xsl:attribute>
	    <xsl:attribute name="height">
	      <xsl:value-of select="$l1 div $level-normalization-factor"/>
	    </xsl:attribute>
	    <xsl:attribute name="stroke">grey</xsl:attribute>
	    <xsl:attribute name="stroke-width">0.5</xsl:attribute>
	    <xsl:attribute name="fill">yellow</xsl:attribute>
	    <xsl:attribute name="fill-opacity">0.5</xsl:attribute>
	  </xsl:element>
	</xsl:if>
	<xsl:if test="$l2 &gt; $l1">
<!--
	  <xsl:message>
debug: green rect with x = <xsl:value-of select="(xs:decimal($in_use/src:interval[position()=$i]/@start) div $time-normalization-factor) * $chart-unit-size"/>, width = <xsl:value-of select="((xs:decimal($in_use/src:interval[position()=$i]/@end) - xs:decimal($in_use/src:interval[position()=$i]/@start)) div $time-normalization-factor) * $chart-unit-size"/>, y = <xsl:value-of select="($init-level - $l2) div $level-normalization-factor"/>, height = <xsl:value-of select="($l2 - $l1) div $level-normalization-factor"/>
	  </xsl:message>
-->
	  <xsl:element name="svg:rect">
	    <xsl:attribute name="x">
	      <xsl:value-of select="(xs:decimal($in_use/src:interval[position()=$i]/@start) div $time-normalization-factor) * $chart-unit-size"/>
	    </xsl:attribute>
	    <xsl:attribute name="width">
	      <xsl:value-of select="((xs:decimal($in_use/src:interval[position()=$i]/@end) - xs:decimal($in_use/src:interval[position()=$i]/@start)) div $time-normalization-factor) * $chart-unit-size"/>
	    </xsl:attribute>
	    <xsl:attribute name="y">
	      <xsl:value-of select="($init-level - $l2) div $level-normalization-factor"/>
	    </xsl:attribute>
	    <xsl:attribute name="height">
	      <xsl:value-of select="($l2 - $l1) div $level-normalization-factor"/>
	    </xsl:attribute>
	    <xsl:attribute name="stroke">grey</xsl:attribute>
	    <xsl:attribute name="stroke-width">0.5</xsl:attribute>
	    <xsl:attribute name="fill">green</xsl:attribute>
	    <xsl:attribute name="fill-opacity">0.5</xsl:attribute>
	  </xsl:element>
	</xsl:if>
	<xsl:if test="$init-level &gt; $l2">
	  <xsl:element name="svg:rect">
	    <xsl:attribute name="x">
	      <xsl:value-of select="(xs:decimal($in_use/src:interval[position()=$i]/@start) div $time-normalization-factor) * $chart-unit-size"/>
	    </xsl:attribute>
	    <xsl:attribute name="width">
	      <xsl:value-of select="((xs:decimal($in_use/src:interval[position()=$i]/@end) - xs:decimal($in_use/src:interval[position()=$i]/@start)) div $time-normalization-factor) * $chart-unit-size"/>
	    </xsl:attribute>
	    <xsl:attribute name="y">
	      <xsl:value-of select="0"/>
	    </xsl:attribute>
	    <xsl:attribute name="height">
	      <xsl:value-of select="($init-level - $l2) div $level-normalization-factor"/>
	    </xsl:attribute>
	    <xsl:attribute name="stroke">grey</xsl:attribute>
	    <xsl:attribute name="stroke-width">0.5</xsl:attribute>
	    <xsl:attribute name="fill">red</xsl:attribute>
	    <xsl:attribute name="fill-opacity">0.5</xsl:attribute>
	  </xsl:element>
	</xsl:if>
      </xsl:for-each>
    </xsl:element>
  </xsl:template>

  <xsl:template name="makeTraitMMU">
    <xsl:choose>
      <xsl:when test="(@min='true') and (@unique='true')">
        <xsl:text> (min, </xsl:text><xsl:element name="xhtml:b"><xsl:text>unique</xsl:text></xsl:element><xsl:text>)</xsl:text>
      </xsl:when>
      <xsl:when test="(@max='true') and (@unique='true')">
        <xsl:text> (max, </xsl:text><xsl:element name="xhtml:b"><xsl:text>unique</xsl:text></xsl:element><xsl:text>)</xsl:text>
      </xsl:when>
      <xsl:when test="(@min='true')">
        <xsl:text> (min)</xsl:text>
      </xsl:when>
      <xsl:when test="(@max='true')">
        <xsl:text> (max)</xsl:text>
      </xsl:when>
      <xsl:when test="@unique='true'">
        <xsl:text> (</xsl:text><xsl:element name="xhtml:b"><xsl:text>unique</xsl:text></xsl:element><xsl:text>)</xsl:text>
      </xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="makeTraitsList">
    <xsl:element name="xhtml:ul">
      <!-- action occurrence traits -->
      <xsl:for-each select="src:traits/src:plan-action-occurs">
	<xsl:element name="xhtml:li">
	  <xsl:choose>
	    <xsl:when test="@count=0">
Action <xsl:value-of select="@name"/> <xsl:element name="xhtml:em"> does not </xsl:element> appear in this plan.
            </xsl:when>
	    <xsl:when test="(@count=1) and (@max='true')">
Action <xsl:value-of select="@name"/> <xsl:element name="xhtml:em"> appears </xsl:element> in this plan.
            </xsl:when>
	    <xsl:when test="(@count=1) and (@max='false')">
Action <xsl:value-of select="@name"/> appears <xsl:element name="xhtml:em"> once </xsl:element> in this plan.
            </xsl:when>
	    <xsl:otherwise>
Action <xsl:value-of select="@name"/> appears <xsl:value-of select="@count"/> times in this plan.
            </xsl:otherwise>
	  </xsl:choose>
	  <xsl:choose>
	    <xsl:when test="(@min='true') and (@count &gt; 0) and (@unique='true')">
	      <xsl:text> (min, </xsl:text><xsl:element name="xhtml:b">unique</xsl:element><xsl:text>)</xsl:text>
	    </xsl:when>
	    <xsl:when test="(@max='true') and (@count &gt; 1) and (@unique='true')">
	      <xsl:text> (max, </xsl:text><xsl:element name="xhtml:b">unique</xsl:element><xsl:text>)</xsl:text>
	    </xsl:when>
	    <xsl:when test="(@min='true') and (@count &gt; 0)">
	      <xsl:text> (min)</xsl:text>
	    </xsl:when>
	    <xsl:when test="(@max='true') and (@count &gt; 1)">
	      <xsl:text> (max)</xsl:text>
	    </xsl:when>
	    <xsl:when test="@unique='true'">
	      <xsl:text> (</xsl:text><xsl:element name="xhtml:b">unique</xsl:element><xsl:text>)</xsl:text>
	    </xsl:when>
	  </xsl:choose>
	</xsl:element>
      </xsl:for-each>

      <!-- step order traits -->
      <xsl:if test="$list-step-order-traits='true'">
      <xsl:for-each select="src:traits/src:plan-step-order">
        <xsl:for-each select="src:prec-link">
	  <xsl:element name="xhtml:li">
	    <xsl:variable name="from-step">
	      <xsl:value-of select="@from"/>
	    </xsl:variable>
	    <xsl:variable name="to-step">
	      <xsl:value-of select="@to"/>
	    </xsl:variable>
	    <xsl:variable name="from-act">
	      <xsl:value-of select="ancestor::src:schedule/src:steps/src:step[@id=$from-step]/@action"/>
	    </xsl:variable>
	    <xsl:variable name="to-act">
	      <xsl:value-of select="ancestor::src:schedule/src:steps/src:step[@id=$to-step]/@action"/>
	    </xsl:variable>
	    <xsl:element name="xhtml:li">
	      Step #<xsl:value-of select="$from-step"/>. <xsl:value-of select="ancestor::src:schedule/src:actions/src:action[@id=$from-act]/@name"/> preceds step #<xsl:value-of select="$to-step"/>. <xsl:value-of select="ancestor::src:schedule/src:actions/src:action[@id=$to-act]/@name"/>.
	    </xsl:element>
	  </xsl:element>
	</xsl:for-each>
      </xsl:for-each>
      </xsl:if>

      <!-- peak-use -->
      <xsl:for-each select="src:traits/src:plan-feature-value[@type='peak-use']">
        <xsl:element name="xhtml:li">
	  <xsl:choose>
	    <xsl:when test="@value = 0">
Resource <xsl:value-of select="@name"/> is <xsl:element name="xhtml:em"> not used </xsl:element> by this plan.
	    </xsl:when>
	    <xsl:when test="@value &gt; 0">
Peak use of resource <xsl:value-of select="@name"/> is <xsl:value-of select="@value"/>.
            </xsl:when>
	  </xsl:choose>
	  <xsl:call-template name="makeTraitMMU"/>
	</xsl:element>
      </xsl:for-each>

      <!-- tolerable losses -->
      <xsl:for-each select="src:traits/src:plan-feature-value[@type='tolerable-loss']">
	<!-- <xsl:if test="not(@value = 'INF')"> -->
	  <xsl:element name="xhtml:li">
Tolerable loss of resource <xsl:value-of select="@name"/> is <xsl:value-of select="@value"/>.
	    <xsl:call-template name="makeTraitMMU"/>
	  </xsl:element>
	<!-- </xsl:if> -->
      </xsl:for-each>

      <!-- makespan -->
      <xsl:for-each select="src:traits/src:plan-feature-value[@type='makespan']">
	<xsl:element name="xhtml:li">
Makespan is <xsl:value-of select="@value"/>.
	  <xsl:call-template name="makeTraitMMU"/>
	</xsl:element>
      </xsl:for-each>
    </xsl:element> <!-- end ul -->
  </xsl:template>


  <!-- main schedule-to-xhtml template -->
  <xsl:template match="src:schedule">

    <!-- calculate a number of local variables -->
    <xsl:variable name="durations" as="xs:decimal*">
      <xsl:for-each select="src:actions/src:action">
        <xsl:sequence select="xs:decimal(@duration)"/>
      </xsl:for-each>
    </xsl:variable>
    <xsl:variable name="finish-times" as="xs:decimal*">
      <xsl:for-each select="src:steps/src:step">
        <xsl:sequence select="xs:decimal(@finish)"/>
      </xsl:for-each>
    </xsl:variable>
    <xsl:variable name="min-duration" as="xs:decimal">
      <xsl:value-of select="min($durations)"/>
    </xsl:variable>
    <xsl:variable name="latest-finish-time" as="xs:decimal">
      <xsl:value-of select="max($finish-times)"/>
    </xsl:variable>
    <xsl:variable name="time-normalization-factor" as="xs:decimal">
      <xsl:value-of select="$min-duration"/>
    </xsl:variable>
    <xsl:variable name="normalized-makespan" as="xs:decimal">
      <xsl:value-of select="$latest-finish-time div $time-normalization-factor"/>
    </xsl:variable>
    <xsl:variable name="number-of-tracks" as="xs:decimal">
      <xsl:value-of select="xs:decimal(src:steps/@maxtrack) + 1"/>
    </xsl:variable>
    <xsl:variable name="xscale">
      <xsl:value-of select="$chart-display-width div ($normalized-makespan * $chart-unit-size)"/>
    </xsl:variable>
    <xsl:variable name="chart-display-height">
      <xsl:value-of select="$number-of-tracks * $chart-unit-size * $xscale"/>
    </xsl:variable>

    <!-- and display them -->  
    <xsl:message terminate="no">
chart-display-width = <xsl:value-of select="$chart-display-width"/>
chart-unit-size = <xsl:value-of select="$chart-unit-size"/>
resource-profile-display-height = <xsl:value-of select="$resource-profile-display-height"/>
durations = <xsl:value-of select="$durations"/>
count(durations) = <xsl:value-of select="count($durations)"/>
min-duration = <xsl:value-of select="$min-duration"/>
finish-times = <xsl:value-of select="$finish-times"/>
count(finish-times) = <xsl:value-of select="count($finish-times)"/>
latest-finish-time = <xsl:value-of select="$latest-finish-time"/>
time-normalization-factor = <xsl:value-of select="$time-normalization-factor"/>
normalized-makespan = <xsl:value-of select="$normalized-makespan"/>
number-of-tracks = <xsl:value-of select="$number-of-tracks"/>
xscale = <xsl:value-of select="$xscale"/>
chart-display-height = <xsl:value-of select="$chart-display-height"/>
    </xsl:message>

    <!-- begin main html -->
    <xsl:element name="xhtml:html">
      <xsl:element name="xhtml:head">

        <!-- script stuff -->
        <xsl:element name="xhtml:script">
var resource_req_by_step =
[ <xsl:for-each select="src:resources/src:resource">
   <xsl:variable name="res_id"><xsl:value-of select="@id"/></xsl:variable>
 [ <xsl:for-each select="ancestor::src:schedule/src:steps/src:step">
     <xsl:variable name="step_id"><xsl:value-of select="@id"/></xsl:variable>
     <xsl:choose>
       <xsl:when test="ancestor::src:schedule/src:resources/src:resource[@id=$res_id]/src:link[@step=$step_id]">true</xsl:when>
       <xsl:otherwise>false</xsl:otherwise>
     </xsl:choose>
     <xsl:if test="not(position()=last())">, </xsl:if>
   </xsl:for-each> ]
   <xsl:if test="not(position()=last())">, </xsl:if>
</xsl:for-each> ];

var resource_used_by_step =
[ <xsl:for-each select="src:resources/src:resource">
   <xsl:variable name="res_id"><xsl:value-of select="@id"/></xsl:variable>
 [ <xsl:for-each select="ancestor::src:schedule/src:steps/src:step">
     <xsl:variable name="step_id"><xsl:value-of select="@id"/></xsl:variable>
     <xsl:choose>
       <xsl:when test="(ancestor::src:schedule/src:resources/src:resource[@id=$res_id]/src:link[(@step=$step_id) and (@type='use')]) and (ancestor::src:schedule/src:resources/src:resource[@id=$res_id]/src:link[@step=$step_id]/@amount)">
	  <xsl:value-of select="ancestor::src:schedule/src:resources/src:resource[@id=$res_id]/src:link[(@step=$step_id) and (@type='use')]/@amount"/>
	</xsl:when>
       <xsl:when test="(ancestor::src:schedule/src:resources/src:resource[@id=$res_id]/src:link[(@step=$step_id) and (@type='use')]) and not(ancestor::src:schedule/src:resources/src:resource[@id=$res_id]/src:link[@step=$step_id]/@amount)">-1</xsl:when>
       <xsl:otherwise>0</xsl:otherwise>
     </xsl:choose>
     <xsl:if test="not(position()=last())">, </xsl:if>
   </xsl:for-each> ]
   <xsl:if test="not(position()=last())">, </xsl:if>
</xsl:for-each> ];

var resource_consumed_by_step =
[ <xsl:for-each select="src:resources/src:resource">
   <xsl:variable name="res_id"><xsl:value-of select="@id"/></xsl:variable>
 [ <xsl:for-each select="ancestor::src:schedule/src:steps/src:step">
     <xsl:variable name="step_id"><xsl:value-of select="@id"/></xsl:variable>
     <xsl:choose>
       <xsl:when test="(ancestor::src:schedule/src:resources/src:resource[@id=$res_id]/src:link[(@step=$step_id) and (@type='consume')]) and (ancestor::src:schedule/src:resources/src:resource[@id=$res_id]/src:link[@step=$step_id]/@amount)">
	  <xsl:value-of select="ancestor::src:schedule/src:resources/src:resource[@id=$res_id]/src:link[(@step=$step_id) and (@type='consume')]/@amount"/>
	</xsl:when>
       <xsl:when test="(ancestor::src:schedule/src:resources/src:resource[@id=$res_id]/src:link[(@step=$step_id) and (@type='consume')]) and not(ancestor::src:schedule/src:resources/src:resource[@id=$res_id]/src:link[@step=$step_id]/@amount)">-1</xsl:when>
       <xsl:otherwise>0</xsl:otherwise>
     </xsl:choose>
     <xsl:if test="not(position()=last())">, </xsl:if>
   </xsl:for-each> ]
   <xsl:if test="not(position()=last())">, </xsl:if>
</xsl:for-each> ];

var step_action =
[ <xsl:for-each select="src:steps/src:step"> <xsl:value-of select="@action"/><xsl:if test="not(position()=last())">, </xsl:if> </xsl:for-each> ];

var step_prec =
<xsl:choose>
  <xsl:when test="src:traits/src:plan-precedence-relation">
[ <xsl:for-each select="src:traits/src:plan-precedence-relation/src:step">
 [ <xsl:for-each select="src:link[@type='prec']">
     <xsl:value-of select="@step"/>
   <xsl:if test="not(position()=last())">, </xsl:if>
   </xsl:for-each> ]
  <xsl:if test="not(position()=last())">, </xsl:if>
</xsl:for-each> ];
  </xsl:when>
  <xsl:otherwise>
[ <xsl:for-each select="src:steps/src:step">
 [ <xsl:for-each select="src:link[@type='prec']">
     <xsl:value-of select="@step"/>
   <xsl:if test="not(position()=last())">, </xsl:if>
   </xsl:for-each> ]
  <xsl:if test="not(position()=last())">, </xsl:if>
</xsl:for-each> ];
  </xsl:otherwise>
</xsl:choose>

var action_id =
[ <xsl:for-each select="src:actions/src:action">
  <xsl:value-of select="@id"/>
  <xsl:if test="not(position()=last())">, </xsl:if>
</xsl:for-each> ];

var action_dump =
[ <xsl:for-each select="src:actions/src:action">
  <!-- NOT WORKING:
  "<xsl:value-of select="replace(src:dump/text(), '\n', '\\n')"/>"
  -->
  "<xsl:value-of select="@name"/>"
  <xsl:if test="not(position()=last())">, </xsl:if>
</xsl:for-each> ];

<!-- constant script stuff -->
<![CDATA[
var steps_in_table;
var steps_on_chart;
var is_highlighted;
var is_marked;
var resource_profiles;
var resources_in_table;
var keep_focus_on_mouse_out = false;

function init(nsteps, nres) {
  steps_in_table = new Array(nsteps);
  steps_on_chart = new Array(nsteps);
  is_highlighted = new Array(nsteps);
  is_in_focus = new Array(nsteps);
  is_marked = new Array(nsteps);
  for (i=0; i < nsteps; i++) {
    steps_in_table[i] = document.getElementById('tab_step' + i);
    steps_on_chart[i] = document.getElementById('gc_step' + i);
    is_highlighted[i] = false;
    is_in_focus[i] = false;
    is_marked[i] = false;
  }
  resource_profiles = new Array(nres);
  resources_in_table = new Array(nres);
  for (i=0; i < nres; i++) {
    resource_profiles[i] = document.getElementById('gc_res' + i);
    resources_in_table[i] = document.getElementById('tab_res' + i);
  }
}

function highlight_step_on_chart(stepid) {
  // steps_on_chart[stepid].setAttribute("stroke", "blue");
  // steps_on_chart[stepid].setAttribute("stroke-width", 2);
  // steps_on_chart[stepid].setAttribute("fill", "blue");
  steps_on_chart[stepid].setAttribute("fill-opacity", 0.3);
}

function highlight_step_on_chart_alt_color(stepid, color) {
  steps_on_chart[stepid].setAttribute("fill", color);
  steps_on_chart[stepid].setAttribute("fill-opacity", 0.3);
}

function highlight_successors(stepid) {
  for (i=0; i < step_prec[stepid].length; i++) {
    highlight_step_on_chart_alt_color(step_prec[stepid][i], "deeppink");
    is_highlighted[step_prec[stepid][i]] = true;
  }
}

function highlight_predecessors(stepid) {
  for (i=0; i < step_prec.length; i++) {
    var is_pred = false;
    for (j=0; j < step_prec[i].length; j++) {
      if (step_prec[i][j] == stepid) is_pred = true;
    }
    if (is_pred) {
      highlight_step_on_chart_alt_color(i, "darkviolet");
      is_highlighted[i] = true;
    }
  }
}

function highlight_step_in_table(stepid) {
  steps_in_table[stepid].setAttribute("bgcolor", "grey");
  steps_in_table[stepid].setAttribute("fgcolor", "white");
}

function lowlight_step_on_chart(stepid) {
  // steps_on_chart[stepid].setAttribute("stroke", "black");
  // steps_on_chart[stepid].setAttribute("stroke-width", 1);
  steps_on_chart[stepid].setAttribute("fill", "black");
  steps_on_chart[stepid].setAttribute("fill-opacity", 0);
}

function lowlight_all_steps() {
  for (i=0; i < is_highlighted.length; i++) {
    if (is_highlighted[i]) {
      lowlight_step_on_chart(i);
      lowlight_step_in_table(i);
      is_highlighted[i] = false;
    }
  }
}

function unfocus_all_steps() {
  for (i=0; i < is_in_focus.length; i++)
    is_in_focus[i] = false;
}

function lowlight_step_in_table(stepid) {
  steps_in_table[stepid].setAttribute("bgcolor", "white");
  steps_in_table[stepid].setAttribute("fgcolor", "black");
}

function mark_step_on_chart(stepid) {
  steps_on_chart[stepid].setAttribute("stroke-width", 3);
  // doesn't work: steps_on_chart[stepid].style.strokeWidth = 3;
}

function mark_step_in_table(stepid) {
  steps_in_table[stepid].style.fontWeight = "bold";
}

function unmark_step_on_chart(stepid) {
  steps_on_chart[stepid].setAttribute("stroke-width", 1);
  // doesn't work: steps_on_chart[stepid].style.strokeWidth = 1;
}

function unmark_step_in_table(stepid) {
  steps_in_table[stepid].style.fontWeight = "normal";
}

function unmark_all_steps() {
  for (i=0; i < is_marked.length; i++) {
    if (is_marked[i]) {
      unmark_step_on_chart(i);
      unmark_step_in_table(i);
      is_marked[i] = false;
    }
  }
}

function gantt_chart_mouse_in(stepid) {
  if (!keep_focus_on_mouse_out) {
    lowlight_all_steps();
    highlight_step_on_chart(stepid);
    highlight_successors(stepid);
    highlight_step_in_table(stepid);
    is_highlighted[stepid] = true;
    is_in_focus[stepid] = true;
  }
}

function gantt_chart_mouse_out(stepid) {
  if (!keep_focus_on_mouse_out) {
    lowlight_step_on_chart(stepid);
    lowlight_step_in_table(stepid);
    is_highlighted[stepid] = false;
    is_in_focus[stepid] = false;
  }
}

function gantt_chart_mouse_click(stepid) {
  if (is_in_focus[stepid]) {
    lowlight_all_steps();
    unfocus_all_steps();
  }
  else {
    lowlight_all_steps();
    unfocus_all_steps();
    highlight_step_on_chart(stepid);
    highlight_successors(stepid);
    highlight_predecessors(stepid);
    highlight_step_in_table(stepid);
    is_highlighted[stepid] = true;
    is_in_focus[stepid] = true;
  }
}

function plan_table_mouse_click(stepid) {
  if (is_in_focus[stepid]) {
    for (i=0; i < action_id.length; i++) {
      if (action_id[i] == step_action[stepid]) {
        alert(action_dump[i]);
	return;
      }
    }
    alert("error: no definition of action " + step_action[stepid] + " found");
  }
  else {
    lowlight_all_steps();
    unfocus_all_steps();
    highlight_step_on_chart(stepid);
    highlight_successors(stepid);
    highlight_predecessors(stepid);
    highlight_step_in_table(stepid);
    is_highlighted[stepid] = true;
    is_in_focus[stepid] = true;
  }
}

function hide_all_resources() {
  for (i=0; i < resource_profiles.length; i++) {
    resource_profiles[i].setAttribute("visibility", "hidden");
  }
}

function show_resource(resid) {
  resource_profiles[resid].setAttribute("visibility", "visible");
}

function highlight_resource_in_table(resid) {
  for (i=0; i < resource_profiles.length; i++) {
    resources_in_table[i].setAttribute("bgcolor", "white");
  }
  resources_in_table[resid].setAttribute("bgcolor", "grey");
}

function resource_table_mouse_click(resid) {
  hide_all_resources();
  show_resource(resid);
  highlight_resource_in_table(resid);
  unmark_all_steps();
  for (i=0; i < steps_on_chart.length; i++) {
    if (resource_req_by_step[resid][i]) {
      mark_step_on_chart(i);
      mark_step_in_table(i);
      is_marked[i] = true;
    }
    var amt_used_cell = document.getElementById('step' + i + '_use');
    if (amt_used_cell == null) {
      alert('Element with id ' + 'step' + i + '_use' + ' not found');
    }
    else {
      if ((resource_used_by_step[resid][i] < 0) || (resource_consumed_by_step[resid][i] < 0))  {
        amt_used_cell.innerHTML = "?";
      }
      else if ((resource_used_by_step[resid][i] + resource_consumed_by_step[resid][i]) == 0) {
        amt_used_cell.innerHTML = "";
      }
      else {
        amt_used_cell.innerHTML = (resource_used_by_step[resid][i] + resource_consumed_by_step[resid][i]);
      }
    }
    var amt_cons_cell = document.getElementById('step' + i + '_cons');
    if (amt_cons_cell == null) {
      alert('Element with id ' + 'step' + i + '_cons' + ' not found');
    }
    else {
      if (resource_consumed_by_step[resid][i] < 0) {
        amt_cons_cell.innerHTML = "?";
      }
      else if (resource_consumed_by_step[resid][i] == 0) {
        amt_cons_cell.innerHTML = "";
      }
      else {
        amt_cons_cell.innerHTML = resource_consumed_by_step[resid][i];
      }
    }
  }
}
]]>
        </xsl:element> <!-- end xhtml:script -->
      </xsl:element> <!-- end xhtml:head -->

      <xsl:element name="xhtml:body">
        <xsl:attribute name="onload">init(<xsl:value-of select="count(src:steps/src:step)"/>, <xsl:value-of select="count(src:resources/src:resource)"/>)</xsl:attribute>

        <xsl:element name="svg:defs">
	  <xsl:call-template name="makeGanttChart">
	    <xsl:with-param name="time-normalization-factor">
	      <xsl:value-of select="$time-normalization-factor"/>
	    </xsl:with-param>
	  </xsl:call-template>
	  <xsl:call-template name="makeTimeGrid">
	    <xsl:with-param name="time-normalization-factor">
	      <xsl:value-of select="$time-normalization-factor"/>
	    </xsl:with-param>
	  </xsl:call-template>
	  <xsl:for-each select="src:resources/src:resource">
<!--
	    <xsl:message>
debug: writing profile for resource <xsl:value-of select="@name"/>
	    </xsl:message>
-->
	    <xsl:call-template name="makeProfile3">
	      <xsl:with-param name="id">res<xsl:value-of select="@id"/>_profile</xsl:with-param>
	      <xsl:with-param name="avail">
		<xsl:copy-of select="src:profile[@name='available']/*"/>
	      </xsl:with-param>
	      <xsl:with-param name="in_use">
		<xsl:copy-of select="src:profile[@name='in_use']/*"/>
	      </xsl:with-param>
	      <xsl:with-param name="init-level" select="@initial-capacity"/>
	      <xsl:with-param name="time-normalization-factor">
	        <xsl:value-of select="$time-normalization-factor"/>
	      </xsl:with-param>
	    </xsl:call-template>
	  </xsl:for-each>
	</xsl:element>

	<xsl:element name="xhtml:center">

	  <xsl:element name="xhtml:h2">
	    <xsl:text>Plan #</xsl:text><xsl:value-of select="@id"/>
	    <xsl:if test="@name">
	      (<xsl:value-of select="@name"/>)
	    </xsl:if>
	  </xsl:element>

	  <!-- Graphics: Gantt & resource charts -->

	  <xsl:element name="svg:svg">
	    <xsl:attribute name="width"><xsl:value-of select="$chart-display-width + 20"/>px</xsl:attribute>
	    <xsl:attribute name="height"><xsl:value-of select="$chart-display-height + 20 + $resource-profile-display-height"/>px</xsl:attribute>

	    <xsl:element name="svg:g">
	      <xsl:attribute name="transform">translate(10,5)</xsl:attribute>

	      <xsl:element name="svg:use">
	        <xsl:attribute name="x">0</xsl:attribute>
		<xsl:attribute name="y">0</xsl:attribute>
		<xsl:attribute name="transform">scale(<xsl:value-of select="$xscale"/>)</xsl:attribute>
		<xsl:attribute name="xlink:href">#GanttChart</xsl:attribute>
		<xsl:attribute name="stroke">black</xsl:attribute>
		<xsl:attribute name="stroke-width">1</xsl:attribute>
		<xsl:attribute name="fill">black</xsl:attribute>
		<xsl:attribute name="fill-opacity">0</xsl:attribute>
	      </xsl:element>

	      <xsl:element name="svg:use">
	        <xsl:attribute name="x">0</xsl:attribute>
		<xsl:attribute name="y">0</xsl:attribute>
		<xsl:attribute name="transform">scale(<xsl:value-of select="$xscale"/>, <xsl:value-of select="($chart-display-height + 10 + $resource-profile-display-height) div $chart-unit-size"/>)</xsl:attribute>
		<xsl:attribute name="xlink:href">#TimeGrid</xsl:attribute>
		<xsl:attribute name="stroke">grey</xsl:attribute>
		<xsl:attribute name="stroke-width">0.5</xsl:attribute>
		<xsl:attribute name="fill">none</xsl:attribute>
	      </xsl:element>

	    </xsl:element>

	    <xsl:element name="svg:path">
	      <xsl:attribute name="d">M 0, <xsl:value-of select="$chart-display-height + 15"/> L <xsl:value-of select="$chart-display-width + 20"/>, <xsl:value-of select="$chart-display-height + 15"/></xsl:attribute>
	      <xsl:attribute name="fill">none</xsl:attribute>
	      <xsl:attribute name="stroke">grey</xsl:attribute>
	      <xsl:attribute name="stroke-width">0.5</xsl:attribute>
	    </xsl:element>
	    <xsl:element name="svg:path">
	      <xsl:attribute name="d">M 0, <xsl:value-of select="$chart-display-height + $resource-profile-display-height + 15"/> L <xsl:value-of select="$chart-display-width + 20"/>, <xsl:value-of select="$chart-display-height + $resource-profile-display-height + 15"/></xsl:attribute>
	      <xsl:attribute name="fill">none</xsl:attribute>
	      <xsl:attribute name="stroke">grey</xsl:attribute>
	      <xsl:attribute name="stroke-width">0.5</xsl:attribute>
	    </xsl:element>

	    <xsl:for-each select="src:resources/src:resource">
	      <xsl:element name="svg:g">
	        <xsl:attribute name="id">gc_res<xsl:value-of select="@id"/></xsl:attribute>
	        <xsl:attribute name="visibility">hidden</xsl:attribute>
		<xsl:attribute name="transform">translate(10, <xsl:value-of select="$chart-display-height + 15"/>)</xsl:attribute>

		<xsl:element name="svg:use">
	          <xsl:attribute name="x">0</xsl:attribute>
		  <xsl:attribute name="y">0</xsl:attribute>
		  <xsl:attribute name="transform">scale(<xsl:value-of select="$xscale"/>, 1.0)</xsl:attribute>
		  <xsl:attribute name="xlink:href">#res<xsl:value-of select="@id"/>_profile</xsl:attribute>
		</xsl:element>
	      </xsl:element>
	    </xsl:for-each>

	  </xsl:element> <!-- end svg:svg -->

	  <!-- plan table -->

	  <xsl:element name="xhtml:table">
	    <xsl:attribute name="border">1</xsl:attribute>
	    <xsl:element name="xhtml:tr">
	      <xsl:element name="xhtml:td"/>
	      <xsl:element name="xhtml:td"/>
	      <xsl:element name="xhtml:td"/>
	      <xsl:element name="xhtml:td">
	        <xsl:text>Req.</xsl:text>
	      </xsl:element>
	      <xsl:element name="xhtml:td">
	        <xsl:text>Loss</xsl:text>
	      </xsl:element>
	    </xsl:element>
	    <xsl:for-each select="src:steps/src:step">
	      <xsl:variable name="step_action_id" select="@action"/>
	      <xsl:element name="xhtml:tr">
	        <xsl:attribute name="id">tab_step<xsl:value-of select="@id"/></xsl:attribute>
	        <xsl:attribute name="onclick">plan_table_mouse_click(<xsl:value-of select="@id"/>)</xsl:attribute>
		<xsl:element name="xhtml:td">
		  <xsl:text>#</xsl:text>
		  <xsl:value-of select="@id"/>
		</xsl:element>
		<xsl:element name="xhtml:td">
		  <xsl:text>[</xsl:text>
		  <xsl:value-of select="@start"/>
		  <xsl:text>,</xsl:text>
		  <xsl:value-of select="@finish"/>
		  <xsl:text>]</xsl:text>
		</xsl:element>
		<xsl:element name="xhtml:td">
		  <xsl:element name="xhtml:font">
		    <xsl:attribute name="id">tab_step<xsl:value-of select="@id"/>_font</xsl:attribute>
		    <xsl:attribute name="font-weight">regular</xsl:attribute>
		    <xsl:for-each select="ancestor::src:schedule/src:actions/src:action[@id=$step_action_id]">
		      <xsl:value-of select="@name"/>
		    </xsl:for-each>
		  </xsl:element>
		</xsl:element>
		<xsl:element name="xhtml:td">
		  <xsl:attribute name="id">step<xsl:value-of select="@id"/>_use</xsl:attribute>
		  <xsl:attribute name="align">center</xsl:attribute>
		  <xsl:text></xsl:text>
		</xsl:element>
		<xsl:element name="xhtml:td">
		  <xsl:attribute name="id">step<xsl:value-of select="@id"/>_cons</xsl:attribute>
		  <xsl:attribute name="align">center</xsl:attribute>
		  <xsl:text></xsl:text>
		</xsl:element>
	      </xsl:element>
	    </xsl:for-each>
	  </xsl:element>

	  <!-- errors and warnings (tables) -->

	  <xsl:if test="src:errors/src:error[not(@severity='warning')]">
	    <xsl:element name="xhtml:h2">
	      <xsl:text>Errors</xsl:text>
	    </xsl:element>

	    <xsl:element name="xhtml:table">
	      <xsl:attribute name="border">1</xsl:attribute>
	      <xsl:for-each select="src:errors/src:error[not(@severity='warning')]">
	        <xsl:element name="xhtml:tr">
		  <xsl:attribute name="id">tab_error<xsl:value-of select="@id"/></xsl:attribute>
		  <xsl:element name="xhtml:td">
		    <xsl:text>[</xsl:text>
		    <xsl:value-of select="@time"/>
		    <xsl:text>]</xsl:text>
		  </xsl:element>
		  <xsl:element name="xhtml:td">
		    <xsl:value-of select="@severity"/>
		  </xsl:element>
		  <xsl:element name="xhtml:td">
		    <xsl:value-of select="@type"/>
		  </xsl:element>
		  <xsl:element name="xhtml:td">
		    <xsl:copy-of select="src:dump/text()"/>
		  </xsl:element>
	        </xsl:element>
	      </xsl:for-each>
	    </xsl:element>
	  </xsl:if>


	  <xsl:if test="src:errors/src:error[@severity='warning']">
	    <xsl:element name="xhtml:h2">
	      <xsl:text>Warnings</xsl:text>
	    </xsl:element>

	    <xsl:element name="xhtml:table">
	      <xsl:attribute name="border">1</xsl:attribute>
	      <xsl:for-each select="src:errors/src:error[not(@severity='warning')]">
	        <xsl:element name="xhtml:tr">
		  <xsl:attribute name="id">tab_warning<xsl:value-of select="@id"/></xsl:attribute>
		  <xsl:element name="xhtml:td">
		    <xsl:text>[</xsl:text>
		    <xsl:value-of select="@time"/>
		    <xsl:text>]</xsl:text>
		  </xsl:element>
		  <xsl:element name="xhtml:td">
		    <xsl:copy-of select="src:dump/text()"/>
		  </xsl:element>
	        </xsl:element>
	      </xsl:for-each>
	    </xsl:element>
	  </xsl:if>

	  <!-- distinguishing traits table -->

	  <xsl:if test="src:traits/src:plan-feature-value|src:traits/src:plan-action-occurs|src:traits/src:plan-step-order">

	    <xsl:element name="xhtml:h2">
	      <xsl:text>Plan Features</xsl:text>
	    </xsl:element>

	    <xsl:element name="xhtml:table">
	      <xsl:attribute name="border">1</xsl:attribute>
	      <xsl:element name="xhtml:tr">
	        <xsl:element name="xhtml:td">
		  <xsl:attribute name="align">left</xsl:attribute>
		  <xsl:call-template name="makeTraitsList"/>
		</xsl:element>
	      </xsl:element>
	    </xsl:element> <!-- end table -->
	  </xsl:if>

	  <!-- resource table -->

	  <xsl:element name="xhtml:h2">
	    <xsl:text>Resources</xsl:text>
	  </xsl:element>

	  <xsl:element name="xhtml:table">
	    <xsl:attribute name="border">1</xsl:attribute>
	    <xsl:element name="xhtml:tr">
	      <xsl:element name="xhtml:td">
	        <xsl:text>Name</xsl:text>
	      </xsl:element>
	      <xsl:element name="xhtml:td">
	        <xsl:text>Capacity</xsl:text>
	      </xsl:element>
	      <xsl:element name="xhtml:td">
	        <xsl:text>Peak Use</xsl:text>
	      </xsl:element>
	      <xsl:element name="xhtml:td">
	        <xsl:text>Tolerable Loss</xsl:text>
	      </xsl:element>
	    </xsl:element>
	    <xsl:for-each select="src:resources/src:resource">
	      <xsl:element name="xhtml:tr">
	        <xsl:attribute name="id">tab_res<xsl:value-of select="@id"/></xsl:attribute>
	        <xsl:attribute name="onclick">resource_table_mouse_click(<xsl:value-of select="@id"/>)</xsl:attribute>
		<xsl:element name="xhtml:td">
		  <xsl:value-of select="@name"/>
		</xsl:element>
		<xsl:element name="xhtml:td">
		  <xsl:attribute name="align">center</xsl:attribute>
		  <xsl:value-of select="@initial-capacity"/>
		</xsl:element>
		<xsl:element name="xhtml:td">
		  <xsl:attribute name="align">center</xsl:attribute>
		  <xsl:value-of select="@peak-use"/> (<xsl:value-of select="@peak-use-percent"/>%)
		</xsl:element>
		<xsl:element name="xhtml:td">
		  <xsl:attribute name="align">center</xsl:attribute>
		  <xsl:value-of select="@tolerable-loss"/>
		</xsl:element>
	      </xsl:element>
	    </xsl:for-each>
	  </xsl:element>

	  <!-- plan derivation table -->

	  <xsl:if test="src:traits/src:derived-from[@plan]">

	    <xsl:element name="xhtml:h2">
	      <xsl:text>Plan History</xsl:text>
	    </xsl:element>

	    <xsl:element name="xhtml:table">
	      <xsl:attribute name="border">1</xsl:attribute>
	      <xsl:element name="xhtml:tr">
	        <xsl:element name="xhtml:td">
		  <xsl:attribute name="align">left</xsl:attribute>
		  <xsl:element name="xhtml:dl">
		    <xsl:for-each select="src:traits/src:derived-from[@plan]">
		      <xsl:element name="xhtml:dt">
		        <xsl:text>Created from </xsl:text>
			<xsl:call-template name="link-to-plan">
			  <xsl:with-param name="id">
			    <xsl:value-of select="@plan"/>
			  </xsl:with-param>
			</xsl:call-template>
			<xsl:text> by additional constraints </xsl:text>
		      </xsl:element>
		      <xsl:element name="xhtml:dd">
		        <xsl:element name="xhtml:ul">
			  <xsl:for-each select="src:prec-link">
			    <xsl:variable name="from-step">
			      <xsl:value-of select="@from"/>
			    </xsl:variable>
			    <xsl:variable name="to-step">
			      <xsl:value-of select="@to"/>
			    </xsl:variable>
			    <xsl:variable name="from-act">
			      <xsl:value-of select="ancestor::src:schedule/src:steps/src:step[@id=$from-step]/@action"/>
			    </xsl:variable>
			    <xsl:variable name="to-act">
			      <xsl:value-of select="ancestor::src:schedule/src:steps/src:step[@id=$to-step]/@action"/>
			    </xsl:variable>
			    <xsl:element name="xhtml:li">
			      #<xsl:value-of select="$from-step"/>. <xsl:value-of select="ancestor::src:schedule/src:actions/src:action[@id=$from-act]/@name"/> &lt; #<xsl:value-of select="$to-step"/>. <xsl:value-of select="ancestor::src:schedule/src:actions/src:action[@id=$to-act]/@name"/>
			    </xsl:element>
			  </xsl:for-each>
		        </xsl:element>
		      </xsl:element>
		    </xsl:for-each>
		  </xsl:element>
		</xsl:element>
	      </xsl:element>
	    </xsl:element> <!-- end table -->
	  </xsl:if>

	</xsl:element> <!-- end xhtml:center -->

      </xsl:element> <!-- end xhtml:body -->
    </xsl:element> <!-- end xhtml:html -->
  </xsl:template>

  <xsl:template name="link-to-plan">
    <xsl:param name="id"/>
    <xsl:element name="xhtml:a">
      <xsl:attribute name="href">
        <xsl:value-of select="concat($basename, '-', $id, '.xhtml')"/>
      </xsl:attribute>
      <xsl:text>Plan #</xsl:text><xsl:value-of select="$id"/>
    </xsl:element>
  </xsl:template>

  <xsl:template match="src:schedule-set">
    <xsl:for-each select="src:schedule">
      <xsl:result-document href="{concat($basename, '-', @id, '.xhtml')}">
        <xsl:apply-templates select="."/>
      </xsl:result-document>
    </xsl:for-each>

    <xsl:element name="xhtml:html">
      <xsl:element name="xhtml:head">
      </xsl:element>
      <xsl:element name="xhtml:body">

	<xsl:element name="xhtml:center">
	  <xsl:element name="xhtml:h2">
	    <xsl:value-of select="count(src:schedule)"/>
	    <xsl:text> Plans</xsl:text>
	  </xsl:element>
	</xsl:element> <!-- end center -->

	<xsl:element name="xhtml:dl">
	  <xsl:for-each select="src:schedule">
	    <xsl:element name="xhtml:dt">
	      <xsl:call-template name="link-to-plan">
	        <xsl:with-param name="id">
		  <xsl:value-of select="@id"/>
		</xsl:with-param>
	      </xsl:call-template>
	      <xsl:if test="@name">
	        <xsl:text> (</xsl:text><xsl:value-of select="@name"/><xsl:text>)</xsl:text>
	      </xsl:if>
	    </xsl:element>

	    <xsl:element name="xhtml:dd">
	      <xsl:call-template name="makeTraitsList"/>
	    </xsl:element> <!-- end dd -->
	  </xsl:for-each>
  	</xsl:element> <!-- end dl -->

	<xsl:element name="xhtml:center">
	  <xsl:element name="xhtml:h2">
	    <xsl:text>Summary of Plan Traits</xsl:text>
	  </xsl:element>
	</xsl:element> <!-- end center -->

	<xsl:element name="xhtml:dl">

	  <!-- action occurs's -->
	  <xsl:for-each-group select="//src:traits/src:plan-action-occurs" group-by="@action">
	    <xsl:variable name="tmp_action_name"><xsl:value-of select="@name"/></xsl:variable>
	    <xsl:element name="xhtml:dt">
	      Action <xsl:value-of select="$tmp_action_name"/>...
	    </xsl:element>
	    <xsl:element name="xhtml:dd">
	      <xsl:element name="xhtml:ul">
	        <xsl:for-each-group select="current-group()" group-by="@count">
		  <xsl:element name="xhtml:li">
		    <xsl:choose>
		      <xsl:when test="@count = 0">
		        <xsl:element name="xhtml:i">
			does not appear
			</xsl:element>
		      </xsl:when>
		      <xsl:otherwise>
		        <xsl:element name="xhtml:i">
			appears
			</xsl:element>
		      </xsl:otherwise>
		    </xsl:choose>
		    <xsl:text> in </xsl:text>
		    <xsl:for-each select="current-group()">
		      <xsl:call-template name="link-to-plan">
		        <xsl:with-param name="id">
			  <xsl:value-of select="ancestor::src:schedule/@id"/>
			</xsl:with-param>
		      </xsl:call-template>
		    </xsl:for-each>
		  </xsl:element>
		</xsl:for-each-group>
	      </xsl:element>
	    </xsl:element> <!-- end dd -->
	  </xsl:for-each-group>

	  <!-- peak use f.v.'s -->
	  <xsl:for-each-group select="//src:traits/src:plan-feature-value[@type='peak-use']" group-by="@resource">
	    <xsl:variable name="tmp_resource"><xsl:value-of select="@resource"/></xsl:variable>
	    <xsl:element name="xhtml:dt">
	      Peak use of resource <xsl:value-of select="ancestor::src:schedule/src:resources/src:resource[@id=$tmp_resource]/@name"/> is...
	    </xsl:element>
	    <xsl:element name="xhtml:dd">
	      <xsl:element name="xhtml:ul">
	        <xsl:for-each-group select="current-group()" group-by="@value">
		  <xsl:variable name="tmp_value"><xsl:value-of select="@value"/></xsl:variable>
		  <xsl:element name="xhtml:li">
		    <xsl:element name="xhtml:b">
		      <xsl:value-of select="@value"/>
		    </xsl:element>
		    <xsl:text> in </xsl:text>
		    <xsl:for-each select="current-group()">
		      <xsl:call-template name="link-to-plan">
		        <xsl:with-param name="id">
			  <xsl:value-of select="ancestor::src:schedule/@id"/>
			</xsl:with-param>
		      </xsl:call-template>
		    </xsl:for-each>
		  </xsl:element>
		</xsl:for-each-group>
	      </xsl:element>
	    </xsl:element> <!-- end dd -->
	  </xsl:for-each-group>

	  <!-- tolerable loss f.v.'s -->
	  <xsl:for-each-group select="//src:traits/src:plan-feature-value[@type='tolerable-loss']" group-by="@resource">
	    <xsl:variable name="tmp_resource"><xsl:value-of select="@resource"/></xsl:variable>
	    <xsl:element name="xhtml:dt">
	      Tolerable loss of resource <xsl:value-of select="//src:schedule[position()=1]/src:resources/src:resource[@id=$tmp_resource]/@name"/> is...
	    </xsl:element>
	    <xsl:element name="xhtml:dd">
	      <xsl:element name="xhtml:ul">
	        <xsl:for-each-group select="current-group()" group-by="@value">
		  <xsl:variable name="tmp_value"><xsl:value-of select="@value"/></xsl:variable>
		  <xsl:element name="xhtml:li">
		    <xsl:element name="xhtml:b">
		      <xsl:value-of select="@value"/>
		    </xsl:element>
		    <xsl:text> in </xsl:text>
		    <xsl:for-each select="current-group()">
		      <xsl:call-template name="link-to-plan">
		        <xsl:with-param name="id">
			  <xsl:value-of select="ancestor::src:schedule/@id"/>
			</xsl:with-param>
		      </xsl:call-template>
		    </xsl:for-each>
		  </xsl:element>
		</xsl:for-each-group>
	      </xsl:element>
	    </xsl:element> <!-- end dd -->
	  </xsl:for-each-group>

	  <!-- makespan f.v.'s -->
	  <xsl:element name="xhtml:dt">
	    Makespan is...
	  </xsl:element>
	  <xsl:choose>
	    <xsl:when test="//src:traits/src:plan-feature-value[@type='makespan']">
	      <xsl:element name="xhtml:dd">
	        <xsl:element name="xhtml:ul">
		  <xsl:for-each-group select="//src:traits/src:plan-feature-value[@type='makespan']" group-by="@value">
		    <xsl:variable name="tmp_value"><xsl:value-of select="@value"/></xsl:variable>
		    <xsl:element name="xhtml:li">
		      <xsl:element name="xhtml:b">
		        <xsl:value-of select="@value"/>
		      </xsl:element>
		      <xsl:text> in </xsl:text>
		      <xsl:for-each select="current-group()">
		        <xsl:call-template name="link-to-plan">
		          <xsl:with-param name="id">
			    <xsl:value-of select="ancestor::src:schedule/@id"/>
			  </xsl:with-param>
		        </xsl:call-template>
		      </xsl:for-each>
		    </xsl:element>
		  </xsl:for-each-group>
		</xsl:element> <!-- end ul -->
	      </xsl:element> <!-- end dd -->
	    </xsl:when>
	    <xsl:otherwise>
	      <xsl:element name="xhtml:dd">
	        <xsl:value-of select="//src:schedule[0]/@makespan"/> in all plans
	      </xsl:element>
	    </xsl:otherwise>
	  </xsl:choose>

<!-- not working
	  <xsl:for-each-group select="//src:traits/src:plan-step-order/src:prec-link" group-by="@from">
	    <xsl:for-each-group select="current-group()" group-by="@to">
	      <xsl:element name="xhtml:dt">
	        <xsl:variable name="from-step">
		  <xsl:value-of select="@from"/>
		</xsl:variable>
		<xsl:variable name="to-step">
		  <xsl:value-of select="@to"/>
		</xsl:variable>
		  <xsl:variable name="from-act">
		<xsl:value-of select="ancestor::src:schedule/src:steps/src:step[@id=$from-step]/@action"/>
		</xsl:variable>
		  <xsl:variable name="to-act">
		<xsl:value-of select="ancestor::src:schedule/src:steps/src:step[@id=$to-step]/@action"/>
		</xsl:variable>
Step #<xsl:value-of select="$from-step"/>. <xsl:value-of select="ancestor::src:schedule/src:actions/src:action[@id=$from-act]/@name"/> preceds step #<xsl:value-of select="$to-step"/>. <xsl:value-of select="ancestor::src:schedule/src:actions/src:action[@id=$to-act]/@name"/> in 
		<xsl:for-each select="current-group()">
		  <xsl:call-template name="link-to-plan">
		    <xsl:with-param name="id">
		      <xsl:value-of select="ancestor::src:schedule/@id"/>
		    </xsl:with-param>
		  </xsl:call-template>
		</xsl:for-each>
	      </xsl:element>
	    </xsl:for-each-group>
	  </xsl:for-each-group>
-->

	</xsl:element> <!-- end dl -->

      </xsl:element> <!-- end body -->
    </xsl:element> <!-- end html -->

  </xsl:template>

  <xsl:template match="/">
    <xsl:apply-templates select="src:schedule|src:schedule-set"/>
  </xsl:template>

</xsl:stylesheet>
