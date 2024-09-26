.. _roq-gate-futures:

.. |checkmark| unicode:: U+2713

roq-gate-futures
================


Links
-----

* `Website <https://www.gate.io/>`__
* `Documentation <https://www.gate.io/api2>`__


Supports
--------

.. grid::  2
  :gutter: 2

  .. grid-item-card::  Products

    .. list-table::
      :widths: auto

      * - Spot
        -
      * - Futures
        - |checkmark|
      * - Options
        -
      * - Combos
        -

  .. grid-item-card::  Market Data

    .. list-table::
      :widths: auto

      * - Reference Data
        - |checkmark|
      * - Market Status
        -
      * - Top of Book
        - |checkmark|
      * - Market by Price
        - |checkmark|
      * - Market by Order
        -
      * - Trade Summary
        - |checkmark|
      * - Statistics
        - |checkmark|

  .. grid-item-card::  Order Management

    .. list-table::
      :widths: auto

      * - Create
        - |checkmark|
      * - Modify
        -
      * - Cancel
        - |checkmark|
      * - Cancel All
        - |checkmark|
      * - Auto-Cancel
        -

  .. grid-item-card::  Account Management

    .. list-table::
      :widths: auto

      * - Positions
        -
      * - Funds
        - |checkmark|


Installing
----------

* :ref:`Using Conda <tutorial-conda>`

.. tab:: Stable

  .. code-block:: shell

     $ mamba install \
           --channel https://roq-trading.com/conda/stable \
           roq-gate-futures

.. tab:: Unstable

  .. code-block:: shell

     $ mamba install \
           --channel https://roq-trading.com/conda/unstable \
           roq-gate-futures


Using
-----

.. code-block:: shell

   $ roq-gate-futures \
         --name "gate-futures" \
         --config_file $CONFIG_FILE_PATH \
         --client_listen_address $UNIX_SOCKET_PATH \
         --flagfile $ENVIRONMENT_FLAGFILE


.. _roq-gate-futures-flags:

Flags
-----

* :ref:`Using Flags <abseil-cpp>`
* :ref:`Gateway Flags <gateway-flags>`

.. code-block:: shell

   $ roq-gate-futures --help

.. tab:: Flags

   .. include:: flags/flags.rstinc

.. tab:: REST

   .. include:: flags/rest.rstinc

.. tab:: WS

   .. include:: flags/ws.rstinc

.. tab:: Download

   .. include:: flags/download.rstinc

.. tab:: Misc

   .. include:: flags/misc.rstinc



Environments
------------

.. tab:: Prod (USDT)

   .. code-block:: shell

      $ $CONDA_PREFIX/share/roq-gate-futures/flags/prod/flags-usdt.cfg

   .. include:: flags/prod/flags-usdt.cfg
     :code: shell

.. tab:: Prod (BTC)

   .. code-block:: shell

      $ $CONDA_PREFIX/share/roq-gate-futures/flags/prod/flags-btc.cfg

   .. include:: flags/prod/flags-btc.cfg
     :code: shell


Configuration
-------------

* :ref:`Gateway Config <gateway-config>`

.. code-block:: shell

   $ $CONDA_PREFIX/share/roq-gate-futures/config.toml

.. important::

   The template will be replaced when the software is upgraded.
   Make a copy and modify to your needs.

.. include:: config.toml
   :code: toml


Market Data
-----------

.. tab:: Live

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::ReferenceData`
      -
      -
      -

    * - :cpp:class:`roq::MarketStatus`
      -
      -
      -

    * - :cpp:class:`roq::TopOfBook`
      - MarketData
      - futures.book_ticker
      -

    * - :cpp:class:`roq::MarketByPriceUpdate`
      - MarketData
      - futures.order_book_update
      -

    * - :cpp:class:`roq::MarketByOrderUpdate`
      -
      -
      -

    * - :cpp:class:`roq::TradeSummary`
      - MarketData
      - futures.trades
      -

    * - :cpp:class:`roq::StatisticsUpdate`
      - MarketData
      - futures.tickers
      -

.. tab:: Download

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::ReferenceData`
      - Rest
      - /futures/{api}/contracts
      -

    * - :cpp:class:`roq::MarketStatus`
      -
      -
      -

    * - :cpp:class:`roq::TopOfBook`
      -
      -
      -

    * - :cpp:class:`roq::MarketByPriceUpdate`
      - Rest
      - /futures/{api}/order_book
      -

    * - :cpp:class:`roq::MarketByOrderUpdate`
      -
      -
      -

    * - :cpp:class:`roq::TradeSummary`
      -
      -
      -

    * - :cpp:class:`roq::StatisticsUpdate`
      -
      -
      -


Statistics
~~~~~~~~~~

.. list-table::
  :header-rows: 1
  :widths: auto

  * - Type
    - Comments

  * - :cpp:class:`TRADE_VOLUME`
    - (futures.tickers) :code:`volume_24h_quote`

  * - :cpp:class:`INDEX_VALUE`
    - (futures.tickers) :code:`index_price`

  * - :cpp:class:`FUNDING_RATE`
    - (futures.tickers) :code:`funding_rate`

  * - :cpp:class:`FUNDING_RATE_PREDICTION`
    - (futures.tickers) :code:`funding_rate_indicative`



Order Management
----------------

.. tab:: Live

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::OrderUpdate`
      -
      -
      -

    * - :cpp:class:`roq::TradeUpdate`
      -
      -
      -

.. tab:: Download

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::OrderUpdate`
      -
      -
      -

    * - :cpp:class:`roq::TradeUpdate`
      -
      -
      -

.. tab:: Request

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::CreateOrder`
      -
      -
      -

    * - :cpp:class:`roq::ModifyOrder`
      -
      -
      -

    * - :cpp:class:`roq::CancelOrder`
      -
      -
      -

    * - :cpp:class:`roq::CancelAllOrders`
      -
      -
      -

.. tab:: Response

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::OrderAck`
      -
      -
      -

Order Types
~~~~~~~~~~~

TBD


Time in Force
~~~~~~~~~~~~~

TBD


Position Effect
~~~~~~~~~~~~~~~

TBD


Execution Instructions
~~~~~~~~~~~~~~~~~~~~~~

TBD


Account Management
------------------

.. tab:: Live

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::PositionUpdate`
      -
      -
      -

    * - :cpp:class:`roq::FundsUpdate`
      -
      -
      -

.. tab:: Download

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::PositionUpdate`
      -
      -
      -

    * - :cpp:class:`roq::FundsUpdate`
      -
      -
      -


Streams
-------

.. tab:: Rest

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Type
      - Comments

    * - REST
      - Primary purpose

        * discover the full list of symbols

.. tab:: MarketData

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Type
      - Comments

    * - WebSocket
      - Primary purpose

        * live market data

        Each connection

        * supports a slice of the symbols



Constraints
-----------

* Order book updates are throttled at 100ms (default) or 1000ms.

* Order books appear to allow inverted prices.

  * This has been confirmed and documented with a request (snapshot) + a single update (which brackets the snapshot).
    Applying the update to the snapshot results in inverted prices.


Comments
--------

* Order books can be subscribed to a depth of 5, 10 and 20, according to the documentation.
  However, a value of 100 also appear to be valid.
  Other values may result in a response saying "success', yet updates do not appear to arrive.

* Order/account management is completely missing (needs sponsorship).
