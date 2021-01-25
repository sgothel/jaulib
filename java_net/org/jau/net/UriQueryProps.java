/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
 * Copyright (c) 2012 Gothel Software e.K.
 * Copyright (c) 2013 JogAmp Community.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
package org.jau.net;

import java.net.URISyntaxException;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;

/**
 * Helper class to process URI's query, handled as properties.
 * <p>
 * The order of the URI segments (any properties) are <i>not</i> preserved.
 * </p>
 * <pre>
 *  URI: [scheme:][//authority][path][?query][#fragment]
 *  w/ authority: [user-info@]host[:port]
 *  Note: 'path' starts w/ fwd slash
 * </pre>
 * <p>
 * Since 2.3.0 renamed from {@code URIQueryProps} to {@code UriQueryProps},
 * and using {@link Uri} instead of {@link java.net.URI}.
 * </p>
 */
public class UriQueryProps {
   private static final String QMARK = "?";
   private static final char ASSIG = '=';
   private static final String EMPTY = "";
   private final String query_separator;

   private final HashMap<String, String> properties = new HashMap<String, String>();

   private UriQueryProps(final char querySeparator) {
       query_separator = String.valueOf(querySeparator);
   }

   public final Map<String, String> getProperties() { return properties; }
   public final char getQuerySeparator() { return query_separator.charAt(0); }

   public final Uri.Encoded appendQuery(Uri.Encoded baseQuery) {
       boolean needsSep = false;
       final StringBuilder sb = new StringBuilder();
       if ( null != baseQuery ) {
           if( baseQuery.startsWith(QMARK) ) {
               baseQuery = baseQuery.substring(1); // cut off '?'
           }
           sb.append(baseQuery.get());
           if( !baseQuery.endsWith(query_separator) ) {
               needsSep = true;
           }
       }
       final Iterator<Entry<String, String>> entries = properties.entrySet().iterator();
       while(entries.hasNext()) {
           if(needsSep) {
               sb.append(query_separator);
           }
           final Entry<String, String> entry = entries.next();
           sb.append(entry.getKey());
           if( EMPTY != entry.getValue() ) {
               sb.append(ASSIG).append(entry.getValue());
           }
           needsSep = true;
       }
       return new Uri.Encoded(sb.toString(), Uri.QUERY_LEGAL);
   }

   public final Uri appendQuery(final Uri base) throws URISyntaxException {
       return base.getNewQuery( appendQuery( base.query ) );
   }

   /**
    *
    * @param uri
    * @param querySeparator should be either <i>;</i> or <i>&</i>, <i>;</i> is encouraged due to troubles of escaping <i>&</i>.
    * @return
    * @throws IllegalArgumentException if <code>querySeparator</code> is illegal, i.e. neither <i>;</i> nor <i>&</i>
    */
   public static final UriQueryProps create(final Uri uri, final char querySeparator) throws IllegalArgumentException {
       if( ';' != querySeparator && '&' != querySeparator ) {
           throw new IllegalArgumentException("querySeparator is invalid: "+querySeparator);
       }
       final UriQueryProps data = new UriQueryProps(querySeparator);
       final String q = Uri.decode(uri.query);
       final int q_l = null != q ? q.length() : -1;
       int q_e = -1;
       while(q_e < q_l) {
           final int q_b = q_e + 1; // next term
           q_e = q.indexOf(querySeparator, q_b);
           if(0 == q_e) {
               // single separator
               continue;
           }
           if(0 > q_e) {
               // end
               q_e = q_l;
           }
           // n-part
           final String part = q.substring(q_b, q_e);
           final int assignment = part.indexOf(ASSIG);
           if(0 < assignment) {
               // assignment
               final String k = part.substring(0, assignment);
               final String v = part.substring(assignment+1);
               data.properties.put(k, v);
           } else {
               // property key only
               data.properties.put(part, EMPTY);
           }
       }
       return data;
   }
}
