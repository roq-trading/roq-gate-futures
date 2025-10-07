.. _roq-gate-futures:

.. |dagger| unicode:: U+2020
.. |double-dagger| unicode:: U+2021
.. |right-arrow| unicode:: U+2192
.. |right-double-arrow| unicode:: U+21D2
.. |left-right-double-arrow| unicode:: U+21D4
.. |check-mark| unicode:: U+2705
.. |cross-mark| unicode:: U+274C
.. |negative-cross-mark| unicode:: U+274E
.. |footnote-1| unicode:: U+2776
.. |footnote-2| unicode:: U+2777
.. |footnote-3| unicode:: U+2778

roq-gate-futures
================


.. tab:: Unstable

  .. code-block:: shell

     $ conda install \
           --channel https://roq-trading.com/conda/unstable \
           roq-gate-futures

.. tab:: Stable

  .. code-block:: shell

     $ conda install \
           --channel https://roq-trading.com/conda/stable \
           roq-gate-futures


Supports
--------

.. grid::  2
  :gutter: 2

  .. grid-item-card::  Products

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:enumerator:`Spot <roq::SecurityType::SPOT>`
        - |cross-mark|
        -
      * - :cpp:enumerator:`Futures <roq::SecurityType::FUTURES>`
        - |check-mark|
        -
      * - :cpp:enumerator:`Swap <roq::SecurityType::SWAP>`
        - |check-mark|
        -
      * - :cpp:enumerator:`Option <roq::SecurityType::OPTION>`
        - |cross-mark|
        -

  .. grid-item-card::  Market Data

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:class:`ReferenceData <roq::ReferenceData>`
        - |check-mark|
        -
      * - :cpp:class:`MarketStatus <roq::MarketStatus>`
        - |cross-mark|
        -
      * - :cpp:class:`TopOfBook <roq::TopOfBook>`
        - |check-mark|
        -
      * - :cpp:class:`MarketByPrice <roq::MarketByPriceUpdate>`
        - |check-mark|
        -
      * - :cpp:class:`MarketByOrder <roq::MarketByOrderUpdate>`
        - |cross-mark|
        -
      * - :cpp:class:`TradeSummary <roq::TradeSummary>`
        - |check-mark|
        -
      * - :cpp:class:`Statistics <roq::StatisticsUpdate>`
        - |check-mark|
        -
      * - :cpp:class:`TimeSeries <roq::TimeSeriesUpdate>`
        - |check-mark|
        -

  .. grid-item-card::  Orders

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:class:`CreateOrder <roq::CreateOrder>`
        - |check-mark|
        -
      * - :cpp:class:`ModifyOrder <roq::ModifyOrder>`
        - |cross-mark|
        -
      * - :cpp:class:`CancelOrder <roq::CancelOrder>`
        - |check-mark|
        -
      * - :cpp:class:`CancelAllOrders <roq::CancelAllOrders>`
        - |check-mark|
        -
      * - :cpp:class:`MassQuote <roq::MassQuote>`
        - |cross-mark|
        -
      * - :cpp:class:`CancelQuotes <roq::CancelQuotes>`
        - |cross-mark|
        -

  .. grid-item-card::  Account

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:class:`Funds <roq::FundsUpdate>`
        - |check-mark|
        -
      * - :cpp:class:`Position <roq::PositionUpdate>`
        - |check-mark|
        -

.. note::

   |check-mark| = Available.

   |negative-cross-mark| = Not implemented.

   |cross-mark| = Unavailable.


Using
-----

.. code-block:: shell

   $ roq-gate-futures [FLAGS]


.. _roq-gate-futures-flags:

Flags
-----

.. code-block:: shell

   $ roq-gate-futures --help

.. tab:: Flags

   .. include:: flags/flags.rstinc

.. tab:: REST

   .. include:: flags/rest.rstinc

.. tab:: WS

   .. include:: flags/ws.rstinc

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

.. tab:: Test (USDT)

   .. code-block:: shell

      $ $CONDA_PREFIX/share/roq-gate-futures/flags/test/flags-usdt.cfg

   .. include:: flags/test/flags-usdt.cfg
     :code: shell

.. tab:: Test (BTC)

   .. code-block:: shell

      $ $CONDA_PREFIX/share/roq-gate-futures/flags/test/flags-btc.cfg

   .. include:: flags/test/flags-btc.cfg
     :code: shell


Configuration
-------------

.. code-block:: shell

   $ $CONDA_PREFIX/share/roq-gate-futures/config.toml

.. important::

   This template will be replaced when the software is upgraded.
   Make a copy and modify to your own needs.

.. include:: config.toml
   :code: toml


Market Data
-----------


Inbound
~~~~~~~

.. tab:: StatisticsType

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left

     * - Event
       - Field
       - Comment
       -
       -

     * - :code:`futures.tickers`
       - :code:`volume_24h_quote`
       -
       - |right-double-arrow|
       - :cpp:enumerator:`TRADE_VOLUME <roq::StatisticsType::TRADE_VOLUME>`

     * - :code:`futures.tickers`
       - :code:`index_price`
       -
       - |right-double-arrow|
       - :cpp:enumerator:`INDEX_VALUE <roq::StatisticsType::INDEX_VALUE>`

     * - :code:`futures.tickers`
       - :code:`funding_rate`
       -
       - |right-double-arrow|
       - :cpp:enumerator:`FUNDING_RATE <roq::StatisticsType::FUNDING_RATE>`

     * - :code:`futures.tickers`
       - :code:`funding_rate_indicative`
       -
       - |right-double-arrow|
       - :cpp:enumerator:`FUNDING_RATE_PREDICTION <roq::StatisticsType::FUNDING_RATE_PREDICTION>`


Order Management
----------------


Inbound
~~~~~~~

.. tab:: TimeInForce

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left

     * - Enum
       -
       -

     * - :code:`GTC`
       - |right-double-arrow|
       - :cpp:enumerator:`GTC <roq::TimeInForce::GTC>`

     * - :code:`IOC`
       - |right-double-arrow|
       - :cpp:enumerator:`IOC <roq::TimeInForce::IOC>`

     * - :code:`FOK`
       - |right-double-arrow|
       - :cpp:enumerator:`FOK <roq::TimeInForce::FOK>`


.. tab:: OrderStatus

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left

     * - :code:`finish_as`
       -
       -

     * - :code:`filled`
       - |right-double-arrow|
       - :cpp:enumerator:`COMPLETED <roq::OrderStatus::COMPLETED>`

     * - :code:`cancelled`
       - |right-double-arrow|
       - :cpp:enumerator:`CANCLED <roq::OrderStatus::CANCLED>`

     * - :code:`liquidated`
       - |right-double-arrow|
       - :cpp:enumerator:`CANCLED <roq::OrderStatus::CANCLED>`

     * - :code:`ioc`
       - |right-double-arrow|
       - :cpp:enumerator:`COMPLETED <roq::OrderStatus::COMPLETED>`

     * - :code:`auto_deleveraged`
       - |right-double-arrow|
       - :cpp:enumerator:`CANCLED <roq::OrderStatus::CANCLED>`

     * - :code:`reduce_only`
       - |right-double-arrow|
       - :cpp:enumerator:`CANCLED <roq::OrderStatus::CANCLED>`

     * - :code:`position_close`
       - |right-double-arrow|
       - :cpp:enumerator:`CANCLED <roq::OrderStatus::CANCLED>`

     * - :code:`stp`
       - |right-double-arrow|
       - :cpp:enumerator:`CANCLED <roq::OrderStatus::CANCLED>`

     * - :code:`_new`
       - |right-double-arrow|
       - :cpp:enumerator:`WORKING <roq::OrderStatus::WORKING>`

     * - :code:`_update`
       - |right-double-arrow|
       - :cpp:enumerator:`WORKING <roq::OrderStatus::WORKING>`

     * - :code:`reduce_out`
       - |right-double-arrow|
       - :cpp:enumerator:`WORKING <roq::OrderStatus::WORKING>`


Outbound
~~~~~~~~

.. tab:: CreateOrder

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left

     * - :cpp:member:`order_type <roq::CreateOrder::order_type>`
       - :cpp:member:`execution_instructions <roq::CreateOrder::execution_instructions>`
       - :cpp:member:`price <roq::CreateOrder::price>`
       - :cpp:member:`stop_price <roq::CreateOrder::stop_price>`
       -
       - :code:`price`
       - :code:`reduce_only`

     * - :cpp:enumerator:`MARKET <roq::OrderType::MARKET>`
       -
       - :code:`NaN`
       - :code:`NaN`
       - |right-double-arrow|
       - |cross-mark|
       -

     * - :cpp:enumerator:`MARKET <roq::OrderType::MARKET>`
       -
       - :code:`NaN`
       - |check-mark|
       - |right-double-arrow|
       - |cross-mark|
       -

     * - :cpp:enumerator:`LIMIT <roq::OrderType::LIMIT>`
       -
       - |check-mark|
       - :code:`NaN`
       - |right-double-arrow|
       - |check-mark|
       -

     * - :cpp:enumerator:`LIMIT <roq::OrderType::LIMIT>`
       -
       - |check-mark|
       - |check-mark|
       - |right-double-arrow|
       - |check-mark|
       -


.. tab:: ModifyOrder

   TBD


.. tab:: CancelOrder

   TBD


.. tab:: CancelAllOrders

   TBD


Comments
--------

* Order book updates are throttled at 100ms (default) or 1000ms.

* Order books appear to allow inverted prices.

  * This has been confirmed and documented with a request (snapshot) + a single update (which brackets the snapshot).
    Applying the update to the snapshot results in inverted prices.

* Order books can be subscribed to a depth of 5, 10 and 20, according to the documentation.
  However, a value of 100 also appear to be valid.
  Other values may result in a response saying "success', yet updates do not appear to arrive.

* Order/account management is completely missing (needs sponsorship).

* Testnet doesn't support downloading of (spot) currencies. Use :code:`--rest_disable_currencies_download=true`.


References
----------

Common
~~~~~~

* :ref:`Using Conda <tutorial-conda>`
* :ref:`Using Flags <abseil-cpp>`
* :ref:`Gateway Flags <gateway-flags>`
* :ref:`Gateway Config <gateway-config>`


Exchange
~~~~~~~~

* `Website <https://www.gate.com/futures/>`__
* `Documentation <https://www.gate.com/docs/developers/apiv4/>`__
